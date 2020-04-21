#include "llvmes/dynarec/compiler.h"

namespace llvmes {

// Only one compiler can exist atm
// This is a static ref to the compiler, a hack to keep the "read/write"
// functions static
static Compiler* s_compiler = nullptr;

void write_memory(int16_t addr, int8_t val)
{
    s_compiler->Write(addr, val);
}

int8_t read_memory(int16_t addr)
{
    return s_compiler->Read(addr);
}

inline void putreg(int8_t r)
{
    printf("%s\n", ToHexString(r).c_str());
}

inline void putchar(int8_t c)
{
    std::cout << c;
}

Compiler::Compiler(AST&& ast, const std::string& program_name)
    : ast(std::move(ast)), c(llvmes::make_unique<Compilation>(program_name))
{
    assert(s_compiler == nullptr);
    s_compiler = this;

    int64 = llvm::Type::getInt64Ty(c->m->getContext());
    int32 = llvm::Type::getInt32Ty(c->m->getContext());
    int16 = llvm::Type::getInt16Ty(c->m->getContext());
    int8 = llvm::Type::getInt8Ty(c->m->getContext());
    int1 = llvm::Type::getInt1Ty(c->m->getContext());
    void_ty = llvm::Type::getVoidTy(c->m->getContext());

    // Create functions
    auto putreg_fn = RegisterFunction({int8}, void_ty, "putreg", (void*)putreg);
    c->putreg_fn = putreg_fn;

    auto putchar_fn =
        RegisterFunction({int8}, void_ty, "putchar", (void*)putchar);
    c->putchar_fn = putchar_fn;

    auto write_fn =
        RegisterFunction({int16, int8}, void_ty, "write", (void*)write_memory);
    c->write_fn = write_fn;

    auto read_fn = RegisterFunction({int16}, int8, "read", (void*)read_memory);
    c->read_fn = read_fn;

    auto main_fn = RegisterFunction({}, int32, "main", nullptr);
    c->main_fn = main_fn;

    // Create initial basicblock
    llvm::BasicBlock* entry =
        llvm::BasicBlock::Create(c->m->getContext(), "entry", main_fn);
    c->builder.SetInsertPoint(entry);

    // Initiate all variables to 0
    c->reg_sp = c->builder.CreateAlloca(int8, 0, "SP");
    c->reg_x = c->builder.CreateAlloca(int8, 0, "X");
    c->reg_y = c->builder.CreateAlloca(int8, 0, "Y");
    c->reg_a = c->builder.CreateAlloca(int8, 0, "A");
    c->status_z = c->builder.CreateAlloca(int1, 0, "Z");
    c->status_n = c->builder.CreateAlloca(int1, 0, "N");
    c->status_v = c->builder.CreateAlloca(int1, 0, "V");
    c->status_c = c->builder.CreateAlloca(int1, 0, "C");
    c->status_i = c->builder.CreateAlloca(int1, 0, "I");
    c->status_b = c->builder.CreateAlloca(int1, 0, "B");
    c->status_u = c->builder.CreateAlloca(int1, 0, "U");
    c->status_d = c->builder.CreateAlloca(int1, 0, "D");

    c->builder.CreateStore(GetConstant8(0), c->reg_x);
    c->builder.CreateStore(GetConstant8(0), c->reg_y);
    c->builder.CreateStore(GetConstant8(0), c->reg_a);

    c->builder.CreateStore(GetConstant1(0), c->status_c);
    c->builder.CreateStore(GetConstant1(0), c->status_v);
    c->builder.CreateStore(GetConstant1(0), c->status_n);
    c->builder.CreateStore(GetConstant1(0), c->status_b);
    c->builder.CreateStore(GetConstant1(0), c->status_u);
    c->builder.CreateStore(GetConstant1(0), c->status_z);
    c->builder.CreateStore(GetConstant1(0), c->status_i);
}

std::function<int()> Compiler::GetMain(bool optimize)
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

void Compiler::PassOne()
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

void Compiler::PassTwo()
{
    c->builder.CreateBr(c->basicblocks["Reset"]);

    for (auto it = ast.begin(); it != ast.end(); ++it) {
        if ((*it)->GetType() == StatementType::Instruction) {
            Instruction& instr = (Instruction&)*(*it);
            CodeGen(instr);

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

    c->builder.CreateRet(GetConstant32(0));
}

void Compiler::Compile()
{
    PassOne();
    PassTwo();
}

}  // namespace llvmes
