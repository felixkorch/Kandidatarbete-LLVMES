#include "llvmes/dynarec/compiler.h"
#include "llvmes/dynarec/disassembler.h"

#define PRINT_A 0x8D, 0x09, 0x20
#define PRINT_X 0x4C, 0x14, 0x00
#define LDY_IMM(V) 0xA0, V
#define INX 0xE8
#define DEY 0x88
#define STX_ZPG_Y(V) 0x96, V
#define STY_ZPG(V) 0x84, V
#define REL(X) 0xFE X
#define LDA_IMM(V) 0xA9, V
#define LDA_ABS(B1, B2) 0xAD, B2, B1
#define BNE(V) 0xD0, REL(V)
#define BEQ(V) 0xF0, REL(V)
#define BMI(V) 0x30, REL(V)
#define BCC(V) 0x90, REL(V)
#define BCS(V) 0xB0, REL(V)
#define BPL(V) 0x10, REL(V)
#define BVC(V) 0x50, REL(V)
#define BVS(V) 0x70, REL(V)

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

std::vector<uint8_t> testBranches{
    // Test BNE
    LDY_IMM(0x02),  // loop
    DEY,
    STY_ZPG(0x00),  // Write Y to address 0
    BNE(-3),        // Br -> Loop
    LDA_ABS(0x00, 0x00),
    PRINT_A,  // 00
    // Test BEQ
    LDY_IMM(0x00),
    DEY,            // loop
    STY_ZPG(0x00),  // Write Y to address 0
    BEQ(-3),        // Br -> Loop
    LDA_ABS(0x00, 0x00),
    PRINT_A,  // FF
    // Test BMI
    LDY_IMM(0x81),
    DEY,            // loop
    STY_ZPG(0x00),  // Write Y to address 0
    BMI(-3),        // Br -> Loop
    LDA_ABS(0x00, 0x00),
    PRINT_A,  // 7F
    // Test BPL
    LDY_IMM(0x01),
    DEY,            // loop
    STY_ZPG(0x00),  // Write Y to address 0
    BPL(-3),        // Br -> Loop
    LDA_ABS(0x00, 0x00),
    PRINT_A,  // FF
    // Test BPL
    LDY_IMM(0x01),
    DEY,            // loop
    STY_ZPG(0x00),  // Write Y to address 0
    BPL(-3),        // Br -> Loop
    LDA_ABS(0x00, 0x00),
    PRINT_A,  // FF
};

std::vector<uint8_t> testLDAXY_zeropageXY{
    0xA0, 0x0A,        // LDY, # 0x0A
    0x8C, 0x10, 0x00,  // STY absolute
    0xA0, 0x05,        // LDY, # 0x05
    0xB6, 0x0B,        // LDX, $(0x0B + Y) = 0x10
    0x8E, 0x00, 0x00,  // STX, $0000
    0xAD, 0x00, 0x00,  // LDA, $0000
    0x8D, 0x09, 0x20,  // Print A - should print 10
};

std::vector<uint8_t> testLDAXY_absoluteXY{
    0xA2, 0x0A,        // LDX, # 0x0A
    0x8E, 0x10, 0x00,  // STX absolute
    0xA2, 0x05,        // LDX, # 0x05
    0xBC, 0x0B, 0x00,  // LDY absolute, $(0x0B + X) = 0x10
    0x8C, 0x00, 0x00,  // STY, $0000
    0xAD, 0x00, 0x00,  // LDA, $0000
    0x8D, 0x09, 0x20,  // Print A - should print 10
};

std::vector<uint8_t> testLDA_indirectXY{
    0xA2, 0x0A,        // LDX, # 0x0A
    0x8E, 0x10, 0x00,  // STX absolute
    0xA2, 0x05,        // LDX, # 0x05
    0xA1, 0x0B,        // LDA, $(0x0B + Y) = 0x10
    0x8E, 0x00, 0x00,  // STX, $0000
    0x8D, 0x09, 0x20,  // Print A - should print 10
};

using namespace llvmes;

int main()
{
    auto d = llvmes::make_unique<Disassembler>(std::move(testLDA_indirectXY));

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
