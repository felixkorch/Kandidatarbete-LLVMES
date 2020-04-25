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
    llvm::Value* reg_idr = nullptr;
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
    llvm::BasicBlock* dynJumpBlock = nullptr;
    llvm::BasicBlock* panicBlock = nullptr;

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
    void AddDynJumpTable();

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
    llvm::Value* GetRAMPtr16(uint16_t addr);

    // Can be called by LLVM on runtime
    void Write(uint16_t addr, uint8_t val) { c->ram[addr] = val; }
    uint16_t Read(uint16_t addr) { return c->ram[addr]; }

    friend void write_memory(int16_t addr, int8_t data);
    friend int8_t read_memory(int16_t addr);

    // These two functions are used to write/read -
    // to addresses that are known on compile-time
    void WriteMemory(uint16_t addr, llvm::Value* v);
    llvm::Value* ReadMemory(uint16_t addr);
    llvm::Value* ReadMemory16(uint16_t addr);

    llvm::Value* GetStackAddress(llvm::Value* sp);
    void StackPush(llvm::Value* v);
    llvm::Value* StackPull();

    void CreateCondBranch(llvm::Value* pred, llvm::BasicBlock* target);
    llvm::BasicBlock* CreateAutoLabel();

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

    // Instructions

    void OP_BIT(llvm::Value* v);
    void OP_AND(llvm::Value* v);
    void OP_EOR(llvm::Value* v);
    void OP_ORA(llvm::Value* v);
    void OP_ASL(llvm::Value* v);
    void OP_LSR(llvm::Value* v, bool s);
    void OP_LSR_A();
    void OP_ADC(llvm::Value* v);
    void OP_JSR(llvm::Value* v);
    void OP_JMP(llvm::Value* v);
    void OP_BNE(llvm::Value* v);
    void OP_BEQ(llvm::Value* v);
    void OP_BMI(llvm::Value* v);
    void OP_BCC(llvm::Value* v);
    void OP_BCS(llvm::Value* v);
    void OP_BPL(llvm::Value* v);
    void OP_BVC(llvm::Value* v);
    void OP_BVS(llvm::Value* v);
    void OP_BRK(llvm::Value* v);
    void OP_LDY(llvm::Value* v);
    void OP_LDA(llvm::Value* v);
    void OP_LDX(llvm::Value* v);
    void OP_INX(llvm::Value* v);
    void OP_INC(llvm::Value* v);
    void OP_DEC(llvm::Value* v);
    void OP_INY(llvm::Value* v);
    void OP_DEY(llvm::Value* v);
    void OP_DEX(llvm::Value* v);
    void OP_NOP(llvm::Value* v);
    void OP_SEI(llvm::Value* v);
    void OP_CLI(llvm::Value* v);
    void OP_CLC(llvm::Value* v);
    void OP_CLD(llvm::Value* v);
    void OP_CLV(llvm::Value* v);
    void OP_PHA(llvm::Value* v);
    void OP_PHP(llvm::Value* v);
    void OP_PLA(llvm::Value* v);
    void OP_PLP(llvm::Value* v);
    void OP_ROL(llvm::Value* v);
    void OP_ROR(llvm::Value* v);
    void OP_RTI(llvm::Value* v);
    void OP_RTS(llvm::Value* v);
    void OP_SBC(llvm::Value* v);
    void OP_SEC(llvm::Value* v);
    void OP_SED(llvm::Value* v);
    void OP_STA(llvm::Value* v);
    void OP_STX(llvm::Value* v);
    void OP_STY(llvm::Value* v);
    void OP_TAX(llvm::Value* v);
    void OP_TAY(llvm::Value* v);
    void OP_TSX(llvm::Value* v);
    void OP_TYA(llvm::Value* v);
    void OP_TXS(llvm::Value* v);
    void OP_TXA(llvm::Value* v);
    void OP_CMP(llvm::Value* v);
    void OP_CPX(llvm::Value* v);
    void OP_CPY(llvm::Value* v);
};
}  // namespace llvmes
