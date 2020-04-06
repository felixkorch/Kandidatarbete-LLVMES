#include "llvmes/dynarec/compiler.h"
#include "llvmes/dynarec/disassembler.h"

#define PRINT_A 0x8D, 0x09, 0x20
#define PRINT_X 0x4C, 0x14, 0x00
#define LDY_IMM(V) 0xA0, V
#define INX 0xE8
#define DEY 0x88
#define STX_ZPG_Y(V) 0x96, V
#define REL(X) 0xFE X
#define BNE(V) 0xD0, REL(V)
#define LDA_ABS(B1, B2) 0xAD, B2, B1

std::vector<uint8_t> program1{
    0xA0, 0x0A,        // LDY, # 0x0A
    0xE8,              // INX -- Begin
    0x88,              // DEY
    0x4C, 0x14, 0x00,  // Print X
    0xD0, 0xF9,        // BNE, Begin
};

std::vector<uint8_t> program2{
    0xA0, 0x0A,        // LDY, # 0x0A
    0xE8,              // INX -- Begin
    0x88,              // DEY
    0xD0, 0xFC,        // BNE, Begin
    0x8E, 0x00, 0x00,  // STX, $0000
    0xAD, 0x00, 0x00,  // LDA, $0000
    0x8D, 0x09, 0x20,  // Print A - should print 10
};

std::vector<uint8_t> program3{
    0x8D, 0x09, 0x20,  // Print A - should print 0
    0x4C, 0x06, 0x80,  // JMP to next instruction
    // 0x6C, 0x1E, 0x80,  // JMP Indirect
    // 0xF0, 0x00,  // BEQ till nästa

    0x8D, 0x09, 0x20,  // Print A - should print 0
    0xA0, 0x18,        // LDY, # 0x1A (22)
    0xE8,              // INX -- Begin
    0x88,              // DEY
    0x8D, 0x0A, 0x20,  // Print X
    0xD0, 0xF9,        // BNE, Begin 15

    0x8E, 0x1E, 0x80,  // STX, #0x05
    0x6C, 0x1E, 0x80,  // JMP Indirect
    // 0x4C, 0x1A, 0x00,  // JMP Abs
    // 01 14
    0xAD, 0x06, 0x00,  // LDA, $0005
    0x8D, 0x09, 0x20,  // Print A - should print 22
    0x0C, 0x80         // Data for JMP Indirect $0204
};

using namespace llvmes;

int main()
{
    auto d = llvmes::make_unique<Disassembler>(std::move(program3));

    AST ast;
    std::vector<uint8_t> ram;
    try {
        ast = d->Disassemble();
        ram = d->GetRAM();
    }
    catch (ParseException& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    auto c = llvmes::make_unique<Compiler>(std::move(ast), "load_store");
    c->SetRAM(std::move(ram));
    c->Compile();

    bool optimized = false;
    c->GetMain(optimized)();

    return 0;
}
