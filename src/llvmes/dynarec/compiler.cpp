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
            if (label_exists && !prev.second->is_branchinstruction)
                c->builder.CreateBr(c->basicblocks[ast.labels[index]]);
        }

        if (label_exists)
            c->builder.SetInsertPoint(c->basicblocks[ast.labels[index]]);

        CodeGen(*instr.second);
        prev = instr;
    }

    c->builder.CreateRet(GetConstant32(0));
}

void Compiler::Compile()
{
    PassOne();
    PassTwo();
}

}  // namespace llvmes
