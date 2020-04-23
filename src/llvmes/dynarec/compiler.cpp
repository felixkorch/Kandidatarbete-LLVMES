#include "llvmes/dynarec/compiler.h"

namespace llvmes {

static Compiler* s_compiler = nullptr;

void write_memory(int16_t addr, int8_t val)
{
    s_compiler->Write(addr, val);
}

int8_t read_memory(int16_t addr)
{
    return s_compiler->Read(addr);
}

void putreg(int8_t r)
{
    printf("%s\n", ToHexString(r).c_str());
}

void putchar(int8_t c)
{
    std::cout << c;
}

void putstatus(int8_t s)
{
    printf("[N: %d V: %d Z: %d C: %d] (%s)\n", (bool)(s & 0x80),
           (bool)(s & 0x40), (bool)(s & 0x02), (bool)(s & 0x01),
           ToHexString((uint8_t)s).c_str());
}

Compiler::Compiler(AST ast, const std::string& program_name)
    : ast(ast), c(llvmes::make_unique<Compilation>(program_name))
{
    assert(s_compiler == nullptr);  // Only one compiler can exist in a program
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

    auto putstatus_fn =
        RegisterFunction({int8}, void_ty, "putstatus", (void*)putstatus);
    c->putstatus_fn = putstatus_fn;

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

Compiler::~Compiler()
{
    for (auto& i : ast.instructions)
        delete i.second;
}

void Compiler::SetRAM(std::vector<uint8_t>&& data)
{
    c->ram = std::move(data);
}

std::vector<uint8_t>& Compiler::GetRAMRef()
{
    return c->ram;
}

void Compiler::SetDumpDir(const std::string& path)
{
    c->jitter.set_external_ir_dump_directory(path);
}

void Compiler::StaticTestZ(int v)
{
    llvm::Constant* z = llvm::ConstantInt::get(int1, v == 0);
    c->builder.CreateStore(z, c->status_z);
}

void Compiler::StaticTestN(int v)
{
    llvm::Constant* n = llvm::ConstantInt::get(int1, v & 0x80);
    c->builder.CreateStore(n, c->status_n);
}

void Compiler::DynamicTestZ(llvm::Value* v)
{
    llvm::Value* is_zero = c->builder.CreateICmpEQ(v, GetConstant8(0), "eq");
    c->builder.CreateStore(is_zero, c->status_z);
}
void Compiler::DynamicTestZ16(llvm::Value* v)
{
    llvm::Value* is_zero = c->builder.CreateICmpEQ(v, GetConstant16(0), "eq");
    c->builder.CreateStore(is_zero, c->status_z);
}
void Compiler::DynamicTestN(llvm::Value* v)
{
    llvm::Constant* c_0x80 = llvm::ConstantInt::get(int8, 0x80);
    llvm::Value* do_and = c->builder.CreateAnd(v, c_0x80);
    llvm::Value* is_negative = c->builder.CreateICmpEQ(do_and, c_0x80);
    c->builder.CreateStore(is_negative, c->status_n);
}
void Compiler::DynamicTestN16(llvm::Value* v)
{
    llvm::Constant* c_0x8000 = llvm::ConstantInt::get(int16, 0x8000);
    llvm::Value* do_and = c->builder.CreateAnd(v, c_0x8000);
    llvm::Value* is_negative = c->builder.CreateICmpEQ(do_and, c_0x8000);
    c->builder.CreateStore(is_negative, c->status_n);
}
void Compiler::DynamicTestCCmp(llvm::Value* v)
{
    llvm::Constant* c_0x0100 = llvm::ConstantInt::get(int16, 0x0100);
    llvm::Value* lessThan = c->builder.CreateICmpULT(v, c_0x0100);
    c->builder.CreateStore(lessThan, c->status_c);
}
// Calculates the ram-address as a constant-expr
llvm::Value* Compiler::GetRAMPtr(uint16_t addr)
{
    llvm::Constant* ram_ptr_value =
        llvm::ConstantInt::get(llvm::Type::getInt64Ty(c->m->getContext()),
                               (int64_t)c->ram.data() + (int64_t)addr);

    return llvm::ConstantExpr::getIntToPtr(
        ram_ptr_value, llvm::PointerType::getUnqual(
                           llvm::Type::getInt8Ty(c->m->getContext())));
}

void Compiler::WriteMemory(uint16_t addr, llvm::Value* v)
{
    llvm::Value* ram_ptr = GetRAMPtr(addr);
    c->builder.CreateStore(v, ram_ptr);
}

llvm::Value* Compiler::ReadMemory(uint16_t addr)
{
    llvm::Value* ram_ptr = GetRAMPtr(addr);
    return c->builder.CreateLoad(ram_ptr);
}

llvm::Value* Compiler::GetStackAddress(llvm::Value* sp)
{
    llvm::Value* sp_16 = c->builder.CreateZExt(sp, int16);
    llvm::Constant* c_0x0100 = llvm::ConstantInt::get(int16, 0x0100);
    return c->builder.CreateOr({sp_16, c_0x0100});
}

void Compiler::StackPush(llvm::Value* v)
{
    // Calculate stack address
    llvm::Value* load_sp =
        c->builder.CreateLoad(c->reg_sp);  // load_sp <- reg_sp
    llvm::Value* sp_addr = GetStackAddress(load_sp);

    // Subtract 1 from stack pointer
    llvm::Constant* c1_8 = llvm::ConstantInt::get(int8, 1);
    load_sp = c->builder.CreateSub(load_sp, c1_8);  // load_sp <- load_sp - 1

    c->builder.CreateStore(load_sp, c->reg_sp);        // reg_sp <- load_sp
    c->builder.CreateCall(c->write_fn, {sp_addr, v});  // [addr] <- v
}

llvm::Value* Compiler::StackPull()
{
    // Calculate stack address
    llvm::Value* load_sp =
        c->builder.CreateLoad(c->reg_sp);  // load_sp <- reg_sp

    // Add 1 to stack pointer
    llvm::Constant* c1_8 = llvm::ConstantInt::get(int8, 1);
    load_sp = c->builder.CreateAdd(load_sp, c1_8);  // load_sp <- load_sp + 1

    llvm::Value* sp_addr = GetStackAddress(load_sp);

    c->builder.CreateStore(load_sp, c->reg_sp);           // reg_sp <- load_sp
    return c->builder.CreateCall(c->read_fn, {sp_addr});  // [addr] <- v
}

void Compiler::CreateCondBranch(llvm::Value* pred, llvm::BasicBlock* target)
{
    std::stringstream auto_label;
    auto_label << "AutoLabel " << auto_labels++;
    llvm::BasicBlock* continue_block = llvm::BasicBlock::Create(
        c->m->getContext(), auto_label.str(), (llvm::Function*)c->main_fn);
    c->builder.CreateCondBr(pred, target, continue_block);
    c->builder.SetInsertPoint(continue_block);
}

llvm::Value* Compiler::AddressModeImmediate(uint16_t operand)
{
    return GetConstant8(operand);
}

llvm::Value* Compiler::AddressModeAbsolute(uint16_t addr)
{
    return GetConstant16(addr);
}

llvm::Value* Compiler::AddressModeAbsoluteX(uint16_t addr)
{
    llvm::Value* load_x = c->builder.CreateLoad(c->reg_x);
    llvm::Value* load_x_16 = c->builder.CreateZExt(load_x, int16);
    llvm::Value* addr_base =
        c->builder.CreateAdd(load_x_16, GetConstant16(addr));
    return addr_base;
}

llvm::Value* Compiler::AddressModeAbsoluteY(uint16_t addr)
{
    llvm::Value* load_y = c->builder.CreateLoad(c->reg_y);
    llvm::Value* load_y_16 = c->builder.CreateZExt(load_y, int16);
    llvm::Value* addr_base =
        c->builder.CreateAdd(load_y_16, GetConstant16(addr));
    return addr_base;
}

llvm::Value* Compiler::AddressModeZeropage(uint16_t addr)
{
    // Zero page addressing only has an 8 bit operand
    return GetRAMPtr(addr);
}

llvm::Value* Compiler::AddressModeZeropageX(uint16_t addr)
{
    llvm::Constant* addr_trunc = GetConstant8(addr);
    llvm::Value* load_x = c->builder.CreateLoad(c->reg_x);
    llvm::Value* target_addr = c->builder.CreateAdd(addr_trunc, load_x);
    return c->builder.CreateZExt(target_addr, int16);
}

llvm::Value* Compiler::AddressModeZeropageY(uint16_t addr)
{
    llvm::Constant* addr_trunc = GetConstant8(addr);
    llvm::Value* load_y = c->builder.CreateLoad(c->reg_y);
    llvm::Value* target_addr = c->builder.CreateAdd(addr_trunc, load_y);
    return c->builder.CreateZExt(target_addr, int16);
}

llvm::Value* Compiler::AddressModeIndirect()
{
    return nullptr;
}

llvm::Value* Compiler::AddressModeIndirectX(uint16_t addr)
{
    llvm::Value* load_x = c->builder.CreateLoad(c->reg_x);
    llvm::Value* addr_base = c->builder.CreateAdd(load_x, GetConstant8(addr));

    // low
    llvm::Value* addr_base_16 = c->builder.CreateZExt(addr_base, int16);
    llvm::Value* addr_low = c->builder.CreateCall(c->read_fn, addr_base_16);
    llvm::Value* addr_low_16 = c->builder.CreateZExt(addr_low, int16);

    // high
    llvm::Value* addr_get_high =
        c->builder.CreateAdd(addr_base, GetConstant8(1));
    llvm::Value* addr_get_high_16 = c->builder.CreateZExt(addr_get_high, int16);
    llvm::Value* addr_high =
        c->builder.CreateCall(c->read_fn, addr_get_high_16);
    llvm::Value* high_addr_16 = c->builder.CreateZExt(addr_high, int16);
    llvm::Value* addr_high_shl = c->builder.CreateShl(high_addr_16, 8);

    llvm::Value* addr_hl_or = c->builder.CreateOr(addr_high_shl, addr_low_16);

    return addr_hl_or;
}

llvm::Value* Compiler::AddressModeIndirectY(uint16_t addr)
{
    llvm::Value* load_y = c->builder.CreateLoad(c->reg_y);
    llvm::Value* load_y_16 = c->builder.CreateZExt(load_y, int16);

    // low
    llvm::Value* addr_low =
        c->builder.CreateCall(c->read_fn, GetConstant16(addr));
    llvm::Value* addr_low_16 = c->builder.CreateZExt(addr_low, int16);

    // high
    llvm::Value* addr_get_high_16 =
        c->builder.CreateAdd(GetConstant16(addr), GetConstant16(1));
    llvm::Value* addr_high =
        c->builder.CreateCall(c->read_fn, addr_get_high_16);
    llvm::Value* addr_high_16 = c->builder.CreateZExt(addr_high, int16);
    llvm::Value* addr_high_shl = c->builder.CreateShl(addr_high_16, 8);

    llvm::Value* addr_hl_or = c->builder.CreateOr(addr_high_shl, addr_low_16);
    llvm::Value* addr_or_with_y = c->builder.CreateAdd(addr_hl_or, load_y_16);

    return addr_or_with_y;
}

llvm::Function* Compiler::RegisterFunction(
    llvm::ArrayRef<llvm::Type*> arg_types, llvm::Type* return_type,
    const std::string& name, void* fn_ptr)
{
    llvm::FunctionType* fn_type =
        llvm::FunctionType::get(return_type, arg_types, false);
    llvm::Function* fn = llvm::Function::Create(
        fn_type, llvm::Function::ExternalLinkage, name, *c->m);

    if (fn_ptr != nullptr)
        c->jitter.add_external_symbol(name, fn_ptr);
    return fn;
}

std::function<int()> Compiler::Compile(bool optimize)
{
    PassOne();
    PassTwo();

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
    for (auto& label : ast.labels) {
        llvm::BasicBlock* bb = llvm::BasicBlock::Create(
            c->m->getContext(), label.second, (llvm::Function*)c->main_fn);
        c->basicblocks[label.second] = bb;
    }
}

void Compiler::PassTwo()
{
    c->builder.CreateBr(c->basicblocks["Reset"]);

    std::pair<uint16_t, Instruction*> prev;

    for (auto& instr : ast.instructions) {
        uint16_t index = instr.first;
        bool label_exists = ast.labels.count(index);

        if (prev.second) {
            if (label_exists && prev.second->op_type != MOS6502::Op::JMP)
                c->builder.CreateBr(c->basicblocks[ast.labels[index]]);
        }

        if (label_exists)
            c->builder.SetInsertPoint(c->basicblocks[ast.labels[index]]);

        CodeGen(*instr.second);
        prev = instr;
    }

    c->builder.CreateRet(GetConstant32(0));
}

}  // namespace llvmes
