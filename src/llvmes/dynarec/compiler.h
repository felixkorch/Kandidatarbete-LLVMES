#pragma once

#include "jitter/jitter.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/TargetSelect.h"
#include "llvmes/dynarec/parser.h"

namespace llvmes {

struct Compilation {
    JITTIR::Jitter jitter;
    std::unique_ptr<llvm::Module> m;
    llvm::IRBuilder<> builder;

    // llvm::Value* ram = nullptr;
    llvm::Value* reg_x = nullptr;
    llvm::Value* reg_y = nullptr;
    llvm::Value* reg_a = nullptr;
    llvm::Value* reg_sp = nullptr;
    llvm::Value* status_v = nullptr;
    llvm::Value* status_n = nullptr;
    llvm::Value* status_c = nullptr;
    llvm::Value* status_z = nullptr;
    llvm::Value* status_i = nullptr;
    llvm::Value* status_b = nullptr;
    llvm::Value* status_u = nullptr;
    llvm::Value* status_d = nullptr;
    llvm::Value* main_fn = nullptr;
    llvm::Value* putreg_fn = nullptr;
    llvm::Value* putchar_fn = nullptr;
    llvm::Value* putstatus_fn = nullptr;
    llvm::Value* write_fn = nullptr;
    llvm::Value* read_fn = nullptr;

    std::vector<uint8_t> ram;

    std::unordered_map<uint16_t, llvm::BasicBlock*> basicblocks;

    Compilation(const std::string& program_name)
        : jitter(),
          m(jitter.create_module(program_name)),
          builder(m->getContext()),
          ram(0x10000)
    {
    }
};

class Compiler {
    AST ast;
    std::unique_ptr<Compilation> c;

    // Common used types and constants
    llvm::Type* int64 = nullptr;
    llvm::Type* int32 = nullptr;
    llvm::Type* int16 = nullptr;
    llvm::Type* int8 = nullptr;
    llvm::Type* int1 = nullptr;
    llvm::Type* void_ty = nullptr;

    int auto_labels = 0;
    uint16_t current_block_address = 0;

   public:
    Compiler(AST ast, const std::string& program_name);
    ~Compiler();

    std::function<int()> Compile(bool optimize);
    std::vector<uint8_t>& GetRAMRef();
    void SetRAM(std::vector<uint8_t>&& data);
    void SetDumpDir(const std::string& path);

   private:
    void CodeGen(Instruction& i);
    void PassOne();
    void PassTwo();

    llvm::Constant* GetConstant1(bool v) { return llvm::ConstantInt::get(int1, v); }
    llvm::Constant* GetConstant8(uint8_t v) { return llvm::ConstantInt::get(int8, v); }
    llvm::Constant* GetConstant16(uint16_t v) { return llvm::ConstantInt::get(int16, v); }
    llvm::Constant* GetConstant32(uint32_t v) { return llvm::ConstantInt::get(int32, v); }
    llvm::Constant* GetConstant64(uint64_t v) { return llvm::ConstantInt::get(int64, v); }

    llvm::Function* RegisterFunction(llvm::ArrayRef<llvm::Type*> arg_types,
                                     llvm::Type* return_type, const std::string& name,
                                     void* fn_ptr = nullptr);

    void StaticTestZ(int v);
    void StaticTestN(int v);
    void DynamicTestZ(llvm::Value* v);
    void DynamicTestZ16(llvm::Value* v);
    void DynamicTestN(llvm::Value* v);
    void DynamicTestN16(llvm::Value* v);
    void DynamicTestCCmp(llvm::Value* v);
    // Calculates the ram-address as a constant-expr
    llvm::Value* GetRAMPtr(uint16_t addr);

    // Can be called by LLVM on runtime
    void Write(uint16_t addr, uint8_t val) { c->ram[addr] = val; }
    uint16_t Read(uint16_t addr) { return c->ram[addr]; }

    friend void write_memory(int16_t addr, int8_t data);
    friend int8_t read_memory(int16_t addr);

    // These two functions are used to write/read -
    // to addresses that are known on compile-time
    void WriteMemory(uint16_t addr, llvm::Value* v);
    llvm::Value* ReadMemory(uint16_t addr);

    llvm::Value* GetStackAddress(llvm::Value* sp);
    void StackPush(llvm::Value* v);
    llvm::Value* StackPull();

    void CreateCondBranch(llvm::Value* pred, llvm::BasicBlock* target);

    llvm::Value* AddressModeImmediate(uint16_t operand);
    llvm::Value* AddressModeAbsolute(uint16_t addr);
    llvm::Value* AddressModeAbsoluteX(uint16_t addr);
    llvm::Value* AddressModeAbsoluteY(uint16_t addr);
    llvm::Value* AddressModeZeropage(uint16_t addr);
    llvm::Value* AddressModeZeropageX(uint16_t addr);
    llvm::Value* AddressModeZeropageY(uint16_t addr);
    llvm::Value* AddressModeIndirect();
    llvm::Value* AddressModeIndirectX(uint16_t addr);
    llvm::Value* AddressModeIndirectY(uint16_t addr);
    llvm::Value* AddressModeImplied();
    llvm::Value* AddressModeAccumulator();
};

}  // namespace llvmes
