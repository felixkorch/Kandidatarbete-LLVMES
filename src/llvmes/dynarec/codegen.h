#pragma once

#include "llvm/IR/DataLayout.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/TargetSelect.h"

#include "jitter/jitter.h"
#include "llvmes/dynarec/disassembler.h"

namespace llvmes {

inline void putreg(int8_t r)
{
    printf("%s\n", ToHexString(r).c_str());
}

inline void putchar(int8_t c)
{
    printf("%c", c);
}

struct Compilation {
    JITTIR::Jitter jitter;
    std::unique_ptr<llvm::Module> m;
    llvm::IRBuilder<> builder;

    llvm::Value* ram = nullptr;
    llvm::Value* reg_x = nullptr;
    llvm::Value* reg_y = nullptr;
    llvm::Value* reg_a = nullptr;
    llvm::Value* status_v = nullptr;
    llvm::Value* status_n = nullptr;
    llvm::Value* status_c = nullptr;
    llvm::Value* status_z = nullptr;
    llvm::Value* main_fn = nullptr;
    llvm::Value* putreg_fn = nullptr;
    llvm::Value* putchar_fn = nullptr;

    std::unordered_map<std::string, llvm::BasicBlock*> basicblocks;

    Compilation(const std::string& program_name)
        : jitter(),
          m(jitter.create_module(program_name)),
          builder(m->getContext())
    {
        jitter.set_external_ir_dump_directory(".");
        jitter.add_external_symbol("putreg", &putreg);
        jitter.add_external_symbol("putchar", &putchar);
    }
};

class Compiler {
    AST ast;
    std::unique_ptr<Compilation> c;

    // Common used types and constants
    llvm::Type* int32 = nullptr;
    llvm::Type* int16 = nullptr;
    llvm::Type* int8 = nullptr;
    llvm::Type* int1 = nullptr;
    llvm::Type* void_ty = nullptr;
    llvm::Value* c0_1 = nullptr;
    llvm::Value* c0_8 = nullptr;
    llvm::Value* c0_32 = nullptr;
    llvm::Value* c1_1 = nullptr;
    llvm::Value* c1_8 = nullptr;

    int auto_labels = 0;

    void CodeGen(Instruction& i);

   public:
    Compiler(AST&& ast, const std::string& program_name)
        : ast(std::move(ast)), c(llvmes::make_unique<Compilation>(program_name))
    {
        int32 = llvm::Type::getInt32Ty(c->m->getContext());
        int16 = llvm::Type::getInt16Ty(c->m->getContext());
        int8 = llvm::Type::getInt8Ty(c->m->getContext());
        int1 = llvm::Type::getInt1Ty(c->m->getContext());
        void_ty = llvm::Type::getVoidTy(c->m->getContext());
        c0_1 = llvm::ConstantInt::get(int1, 0);
        c0_8 = llvm::ConstantInt::get(int8, 0);
        c0_32 = llvm::ConstantInt::get(int32, 0);
        c1_1 = llvm::ConstantInt::get(int1, 1);
        c1_8 = llvm::ConstantInt::get(int8, 1);

        // Declare putreg
        llvm::Type* putreg_result_type = void_ty;
        llvm::Type* putreg_argument_types[1] = {int8};
        bool putreg_vararg = false;

        llvm::FunctionType* putreg_function_type = llvm::FunctionType::get(
            putreg_result_type, putreg_argument_types, putreg_vararg);

        llvm::Function* putreg_function = llvm::Function::Create(
            putreg_function_type, llvm::Function::ExternalLinkage, "putreg",
            *c->m);

        c->putreg_fn = putreg_function;

        // Declare putchar
        llvm::Type* putchar_result_type = void_ty;
        llvm::Type* putchar_argument_types[1] = {int8};
        bool putchar_vararg = false;

        llvm::FunctionType* putchar_function_type = llvm::FunctionType::get(
            putchar_result_type, putchar_argument_types, putchar_vararg);

        llvm::Function* putchar_function = llvm::Function::Create(
            putchar_function_type, llvm::Function::ExternalLinkage, "putchar",
            *c->m);

        c->putchar_fn = putchar_function;

        // Main starts here

        llvm::Type* result_type = int32;
        llvm::Type* argument_types[2] = {};
        bool vararg = false;

        llvm::FunctionType* function_type =
            llvm::FunctionType::get(result_type, {}, vararg);

        llvm::Function* function = llvm::Function::Create(
            function_type, llvm::Function::ExternalLinkage, "main", *c->m);

        c->main_fn = function;

        llvm::BasicBlock* entry =
            llvm::BasicBlock::Create(c->m->getContext(), "entry", function);
        c->builder.SetInsertPoint(entry);

        // c->builder.CreateAlloca(int8, 0, "SP");
        // c->builder.CreateAlloca(int16, 0, "PC");
        c->reg_x = c->builder.CreateAlloca(int8, 0, "X");
        c->reg_y = c->builder.CreateAlloca(int8, 0, "Y");
        c->reg_a = c->builder.CreateAlloca(int8, 0, "A");
        c->status_z = c->builder.CreateAlloca(int1, 0, "Z");
        c->status_n = c->builder.CreateAlloca(int1, 0, "N");
        c->status_v = c->builder.CreateAlloca(int1, 0, "V");
        c->status_c = c->builder.CreateAlloca(int1, 0, "C");

        llvm::Type* array_ty = llvm::ArrayType::get(int8, 0xFFFF);
        c->ram = c->builder.CreateAlloca(array_ty, nullptr, "ram");

        c->builder.CreateStore(c0_8, c->reg_x);
        c->builder.CreateStore(c0_8, c->reg_y);
        c->builder.CreateStore(c0_8, c->reg_a);
    }

    void SetDumpDir(const std::string& path)
    {
        c->jitter.set_external_ir_dump_directory(path);
    }

    std::function<int()> GetMain(bool optimize)
    {
        if (optimize)
            c->jitter.enable_optimize_module(true);
        c->jitter.enable_validate_module(true);

        auto ok = c->jitter.add_module(std::move(c->m));

        if (!ok)
            printf("Compilation failed!\n");
        auto fn_ptr = (int (*)())c->jitter.get_symbol_address("main");
        return std::function<int()>(fn_ptr);
    }

    void StaticTestZ(int v)
    {
        llvm::Constant* z = llvm::ConstantInt::get(int1, v == 0);
        c->builder.CreateStore(z, c->status_z);
    }

    void StaticTestN(int v)
    {
        llvm::Constant* n = llvm::ConstantInt::get(int1, v & 0x80);
        c->builder.CreateStore(n, c->status_n);
    }

    void DynamicTestZ(llvm::Value* v)
    {
        llvm::Value* is_zero = c->builder.CreateICmpEQ(v, c0_8, "eq");
        c->builder.CreateStore(is_zero, c->status_z);
    }
    void DynamicTestN(llvm::Value* v)
    {
        llvm::Constant* c_0x80 = llvm::ConstantInt::get(int8, 0x80);
        llvm::Value* do_and = c->builder.CreateAnd(v, c_0x80);
        llvm::Value* is_negative = c->builder.CreateICmpEQ(do_and, c_0x80);
        c->builder.CreateStore(is_negative, c->status_n);
    }
    void DynamicTestCCmp(llvm::Value* v) 
    {
        llvm::Constant* c_0x0100 = llvm::ConstantInt::get(int16, 0x0100);
        llvm::Value* lesThen = c->builder.CreateICmpULT(v, c_0x0100);
        c->builder.CreateStore(lesThen, c->status_c);
    }

    llvm::Value* GetRAMPtr(uint16_t addr)
    {
        llvm::Value* indices[] = {c0_32, llvm::ConstantInt::get(int32, addr)};
        return c->builder.CreateGEP(c->ram, indices);
    }

    void CreateCondBranch(llvm::Value* pred, llvm::BasicBlock* target)
    {
        std::stringstream auto_label;
        auto_label << "AutoLabel " << auto_labels++;
        llvm::BasicBlock* continue_block = llvm::BasicBlock::Create(
            c->m->getContext(), auto_label.str(), (llvm::Function*)c->main_fn);
        c->builder.CreateCondBr(pred, target, continue_block);
        c->builder.SetInsertPoint(continue_block);
    }

    void PassOne()
    {
        for (const auto& node : ast) {
            if (node->GetType() == StatementType::Label) {
                Label& l = (Label&)*node;
                llvm::BasicBlock* bb = llvm::BasicBlock::Create(
                    c->m->getContext(), l.name, (llvm::Function*)c->main_fn);
                c->basicblocks[l.name] = bb;
            }
        }
    }

    void PassTwo()
    {
        c->builder.CreateBr(c->basicblocks["Reset"]);

        for (auto it = ast.begin(); it != ast.end(); ++it) {
            if ((*it)->GetType() == StatementType::Instruction) {
                Instruction& instr = (Instruction&)*(*it);
                CodeGen(instr);

                // TODO: Don't know if this is a one-case scenario or not
                auto next = std::next(it);
                if (next != ast.end()) {
                    if ((*next)->GetType() == StatementType::Label &&
                        instr.is_branchinstruction == false) {
                        c->builder.CreateBr(
                            c->basicblocks[(*next)->GetAs<Label>().name]);
                    }
                }
            }
            else if ((*it)->GetType() == StatementType::Label) {
                Label& l = (Label&)*(*it);
                c->builder.SetInsertPoint(c->basicblocks[l.name]);
            }
        }

        c->builder.CreateRet(c0_32);
    }

    void Compile()
    {
        PassOne();
        PassTwo();
    }
};

}  // namespace llvmes
