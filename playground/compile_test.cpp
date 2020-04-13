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

std::vector<uint8_t> program1 {
    0xA0, 0x0A,        // LDY, # 0x0A
    0xE8,              // INX -- Begin
    0x88,              // DEY
    0x4C, 0x14, 0x00,  // Print X
    0xD0, 0xF9,        // BNE, Begin
};

std::vector<uint8_t> program2 {
    0xA0, 0x0A,        // LDY, # 0x0A
    0xE8,              // INX -- Begin
    0x88,              // DEY
    0xD0, 0xFC,        // BNE, Begin
    0x8E, 0x00, 0x00,  // STX, $0000
    0xAD, 0x00, 0x00,  // LDA, $0000
    0x8D, 0x09, 0x20,  // Print A - should print 10
};

std::vector<uint8_t> program3 {
    LDY_IMM(0x0A),
    INX,                   // Loop
    DEY,
    STX_ZPG_Y(0x00),       // Write X to address 9-0
    BNE(-4),               // Br -> Loop
    LDA_ABS(0x00, 0x00),
    PRINT_A,               // 10
    LDA_ABS(0x00, 0x01),
    PRINT_A,               // 9
    LDA_ABS(0x00, 0x02),
    PRINT_A,               // 8
    LDA_ABS(0x00, 0x03),
    PRINT_A,               // 7
};

std::vector<uint8_t> cmp_Immidiate{
    0xA9, 0x0A,        // LDA; # 0x0A
    0x8D, 0x09, 0x20,  // Print A - should print 0x0A = 10
    0xC9, 0x0A,        // CMP imidiate #0x0A
    0x8D, 0x0C, 0x20,  // Print status N - should print 00
    0x8D, 0x0D, 0x20,  // Print status C - should print 00
    0x8D, 0x0E, 0x20,  // Print status Z - should print 01
    0xA9, 0x0B,        // LDA; # 0x0B
    0x8D, 0x09, 0x20,  // Print A - should print 0x0B = 11
    0xC9, 0x0A,        // CMP imidiate #0x0A
    0x8D, 0x0C, 0x20,  // Print status N - should print 00
    0x8D, 0x0D, 0x20,  // Print status C - should print 00
    0x8D, 0x0E, 0x20,  // Print status Z - should print 00
    0xA9, 0x02,        // LDA; # 0x02
    0x8D, 0x09, 0x20,  // Print A - should print 0x02 = 2
    0xC9, 0x0A,        // CMP imidiate #0x0A
    0x8D, 0x0C, 0x20,  // Print status N - should print 01
    0x8D, 0x0D, 0x20,  // Print status C - should print 01
    0x8D, 0x0E, 0x20,  // Print status Z - should print 00
};

std::vector<uint8_t> cmp_Zeropage{
    0xA9, 0x0A,        // LDA; # 0x0A
    0x8D, 0x09, 0x20,  // Print A - should print 0x0A = 10
    0x8D, 0x20, 0x00,  // reg_a (=0x0A) to 0x0020 in mem
    0xC5, 0x20,        // cmp zeropage mem 0x0020 
    0x8D, 0x0C, 0x20,  // Print status N - should print 00
    0x8D, 0x0D, 0x20,  // Print status C - should print 00
    0x8D, 0x0E, 0x20,  // Print status Z - should print 01
    0xA9, 0x0B,        // LDA; # 0x0B
    0x8D, 0x09, 0x20,  // Print A - should print 0x0B = 11
    0xC5, 0x20,        // cmp zeropage mem 0x0020
    0x8D, 0x0C, 0x20,  // Print status N - should print 00
    0x8D, 0x0D, 0x20,  // Print status C - should print 00
    0x8D, 0x0E, 0x20,  // Print status Z - should print 00
    0xA9, 0x02,        // LDA; # 0x02
    0x8D, 0x09, 0x20,  // Print A - should print 0x02 = 2
    0xC5, 0x20,        // cmp zeropage mem 0x0020
    0x8D, 0x0C, 0x20,  // Print status N - should print 01
    0x8D, 0x0D, 0x20,  // Print status C - should print 01
    0x8D, 0x0E, 0x20,  // Print status Z - should print 00
};

std::vector<uint8_t> cmp_ZeropageX{
    0xA9, 0x0A,        // LDA; # 0x0A
    0x8D, 0x09, 0x20,  // Print A - should print 0x0A = 10
    0xA2, 0x02,        // LDX; # 0x02
    0x8D, 0x0A, 0x20,  // Print X - should print 0x02 = 2
    0x8D, 0x22, 0x00,  // STA: reg_a (=0x0A) to 0x0022 in mem
    0xD5, 0x20,        // cmp zeropageX mem 0x0020
    0x8D, 0x0C, 0x20,  // Print status N - should print 00
    0x8D, 0x0D, 0x20,  // Print status C - should print 00
    0x8D, 0x0E, 0x20,  // Print status Z - should print 01
    0xA9, 0x0B,        // LDA; # 0x0B
    0x8D, 0x09, 0x20,  // Print A - should print 0x0B = 11
    0xD5, 0x20,        // cmp zeropageX mem 0x0020
    0x8D, 0x0C, 0x20,  // Print status N - should print 00
    0x8D, 0x0D, 0x20,  // Print status C - should print 00
    0x8D, 0x0E, 0x20,  // Print status Z - should print 00
    0xA9, 0x02,        // LDA; # 0x02
    0x8D, 0x09, 0x20,  // Print A - should print 0x02 = 2
    0xD5, 0x20,        // cmp zeropageX mem 0x0020
    0x8D, 0x0C, 0x20,  // Print status N - should print 01
    0x8D, 0x0D, 0x20,  // Print status C - should print 01
    0x8D, 0x0E, 0x20,  // Print status Z - should print 00
    };

std::vector<uint8_t> cmp_Absolute{
    0xA9, 0x0A,        // LDA; # 0x0A
    0x8D, 0x09, 0x20,  // Print A - should print 0x0A = 10
    0x8D, 0x00, 0x20,  // sta reg_a (=0x0A) to 0x2000 in mem
    0xCD, 0x00, 0x20,  // cmp Absolute mem 0x2000
    0x8D, 0x0C, 0x20,  // Print status N - should print 00
    0x8D, 0x0D, 0x20,  // Print status C - should print 00
    0x8D, 0x0E, 0x20,  // Print status Z - should print 01
    0xA9, 0x0B,        // LDA; # 0x0B
    0x8D, 0x09, 0x20,  // Print A - should print 0x0B = 11
    0xCD, 0x00, 0x20,  // cmp Absolute mem 0x2000
    0x8D, 0x0C, 0x20,  // Print status N - should print 00
    0x8D, 0x0D, 0x20,  // Print status C - should print 00
    0x8D, 0x0E, 0x20,  // Print status Z - should print 00
    0xA9, 0x02,        // LDA; # 0x02
    0x8D, 0x09, 0x20,  // Print A - should print 0x02 = 2
    0xCD, 0x00, 0x20,  // cmp Absolute mem 0x2000
    0x8D, 0x0C, 0x20,  // Print status N - should print 01
    0x8D, 0x0D, 0x20,  // Print status C - should print 01
    0x8D, 0x0E, 0x20,  // Print status Z - should print 00
};

std::vector<uint8_t> cmp_AbsoluteX{
    0xA9, 0x0A,        // LDA; # 0x0A
    0x8D, 0x09, 0x20,  // Print A - should print 0x0A = 10
    0xA2, 0x02,        // LDX; # 0x02
    0x8D, 0x0A, 0x20,  // Print X - should print 0x02 = 2
    0x8D, 0x02, 0x20,  // STA: reg_a (=0x0A) to 0x2002 in mem
    0xDD, 0x00, 0x20,  // cmp AbsloluteX mem 0x0020
    0x8D, 0x0C, 0x20,  // Print status N - should print 00
    0x8D, 0x0D, 0x20,  // Print status C - should print 00
    0x8D, 0x0E, 0x20,  // Print status Z - should print 01
    0xA9, 0x0B,        // LDA; # 0x0B
    0x8D, 0x09, 0x20,  // Print A - should print 0x0B = 11
    0xDD, 0x00, 0x20,  // cmp AbsoluteX mem 0x2000
    0x8D, 0x0C, 0x20,  // Print status N - should print 00
    0x8D, 0x0D, 0x20,  // Print status C - should print 00
    0x8D, 0x0E, 0x20,  // Print status Z - should print 00
    0xA9, 0x02,        // LDA; # 0x02
    0x8D, 0x09, 0x20,  // Print A - should print 0x02 = 2
    0xDD, 0x00, 0x20,  // cmp AbsolutX mem 0x2000
    0x8D, 0x0C, 0x20,  // Print status N - should print 01
    0x8D, 0x0D, 0x20,  // Print status C - should print 01
    0x8D, 0x0E, 0x20,  // Print status Z - should print 00
};

std::vector<uint8_t> cmp_AbsoluteY{
    0xA9, 0x0A,        // LDA; # 0x0A
    0x8D, 0x09, 0x20,  // Print A - should print 0x0A = 10
    0xA0, 0x02,        // LDX; # 0x02
    0x8D, 0x0B, 0x20,  // Print Y - should print 0x02 = 2
    0x8D, 0x02, 0x20,  // STA: reg_a (=0x0A) to 0x2002 in mem
    0xD9, 0x00, 0x20,  // cmp AbsloluteY mem 0x0020
    0x8D, 0x0C, 0x20,  // Print status N - should print 00
    0x8D, 0x0D, 0x20,  // Print status C - should print 00
    0x8D, 0x0E, 0x20,  // Print status Z - should print 01
    0xA9, 0x0B,        // LDA; # 0x0B
    0x8D, 0x09, 0x20,  // Print A - should print 0x0B = 11
    0xD9, 0x00, 0x20,  // cmp AbsoluteY mem 0x2000
    0x8D, 0x0C, 0x20,  // Print status N - should print 00
    0x8D, 0x0D, 0x20,  // Print status C - should print 00
    0x8D, 0x0E, 0x20,  // Print status Z - should print 00
    0xA9, 0x02,        // LDA; # 0x02
    0x8D, 0x09, 0x20,  // Print A - should print 0x02 = 2
    0xD9, 0x00, 0x20,  // cmp AbsolutY mem 0x2000
    0x8D, 0x0C, 0x20,  // Print status N - should print 01
    0x8D, 0x0D, 0x20,  // Print status C - should print 01
    0x8D, 0x0E, 0x20,  // Print status Z - should print 00
};

std::vector<uint8_t> cpx_Immidiate{
    0xA2, 0x0A,        // LDX; # 0x0A
    0x8D, 0x0A, 0x20,  // Print X - should print 0x0A = 10
    0xE0, 0x0A,        // CPX imidiate #0x0A
    0x8D, 0x0C, 0x20,  // Print status N - should print 00
    0x8D, 0x0D, 0x20,  // Print status C - should print 00
    0x8D, 0x0E, 0x20,  // Print status Z - should print 01
    0xA2, 0x0B,        // LDX; # 0x0B
    0x8D, 0x0A, 0x20,  // Print X - should print 0x0B = 11
    0xE0, 0x0A,        // CPX imidiate #0x0A
    0x8D, 0x0C, 0x20,  // Print status N - should print 00
    0x8D, 0x0D, 0x20,  // Print status C - should print 00
    0x8D, 0x0E, 0x20,  // Print status Z - should print 00
    0xA2, 0x02,        // LDX; # 0x02
    0x8D, 0x0A, 0x20,  // Print X - should print 0x02 = 2
    0xE0, 0x0A,        // CPX imidiate #0x0A
    0x8D, 0x0C, 0x20,  // Print status N - should print 01
    0x8D, 0x0D, 0x20,  // Print status C - should print 01
    0x8D, 0x0E, 0x20,  // Print status Z - should print 00
};

std::vector<uint8_t> cpx_Zeropage{
    0xA9, 0x0A,        // LDA; # 0x0A
    0x8D, 0x09, 0x20,  // Print A - should print 0x0A = 10
    0x8D, 0x20, 0x00,  // reg_a (=0x0A) to 0x0020 in mem
    0xA2, 0x0A,        // LDX; # 0x0A
    0x8D, 0x0A, 0x20,  // Print X - should print 0x0A = 10
    0xE4, 0x20,        // cpx zeropage mem 0x0020
    0x8D, 0x0C, 0x20,  // Print status N - should print 00
    0x8D, 0x0D, 0x20,  // Print status C - should print 00
    0x8D, 0x0E, 0x20,  // Print status Z - should print 01
    0xA2, 0x0B,        // LDX; # 0x0B
    0x8D, 0x0A, 0x20,  // Print X - should print 0x0B = 11
    0xE4, 0x20,        // cpx zeropage mem 0x0020
    0x8D, 0x0C, 0x20,  // Print status N - should print 00
    0x8D, 0x0D, 0x20,  // Print status C - should print 00
    0x8D, 0x0E, 0x20,  // Print status Z - should print 00
    0xA2, 0x02,        // LDX; # 0x02
    0x8D, 0x0A, 0x20,  // Print X - should print 0x02 = 2
    0xE4, 0x20,        // cpx zeropage mem 0x0020
    0x8D, 0x0C, 0x20,  // Print status N - should print 01
    0x8D, 0x0D, 0x20,  // Print status C - should print 01
    0x8D, 0x0E, 0x20,  // Print status Z - should print 00
};

std::vector<uint8_t> cpx_Absolute{
    0xA9, 0x0A,        // LDA; # 0x0A
    0x8D, 0x09, 0x20,  // Print A - should print 0x0A = 10
    0x8D, 0x00, 0x20,  // sta reg_a (=0x0A) to 0x2000 in mem
    0xA2, 0x0A,        // LDX; # 0x0A
    0x8D, 0x0A, 0x20,  // Print X - should print 0x0A = 10
    0xEC, 0x00, 0x20,  // cpx Absolute mem 0x2000
    0x8D, 0x0C, 0x20,  // Print status N - should print 00
    0x8D, 0x0D, 0x20,  // Print status C - should print 00
    0x8D, 0x0E, 0x20,  // Print status Z - should print 01
    0xA2, 0x0B,        // LDX; # 0x0B
    0x8D, 0x0A, 0x20,  // Print X - should print 0x0B = 11
    0xEC, 0x00, 0x20,  // cpx Absolute mem 0x2000
    0x8D, 0x0C, 0x20,  // Print status N - should print 00
    0x8D, 0x0D, 0x20,  // Print status C - should print 00
    0x8D, 0x0E, 0x20,  // Print status Z - should print 00
    0xA2, 0x02,        // LDX; # 0x02
    0x8D, 0x0A, 0x20,  // Print X - should print 0x02 = 2
    0xEC, 0x00, 0x20,  // cpx Absolute mem 0x2000
    0x8D, 0x0C, 0x20,  // Print status N - should print 01
    0x8D, 0x0D, 0x20,  // Print status C - should print 01
    0x8D, 0x0E, 0x20,  // Print status Z - should print 00
};

std::vector<uint8_t> cpy_Immidiate{
    0xA0, 0x0A,        // LDY; # 0x0A
    0x8D, 0x0B, 0x20,  // Print Y - should print 0x0A = 10
    0xC0, 0x0A,        // CPY imidiate #0x0A
    0x8D, 0x0C, 0x20,  // Print status N - should print 00
    0x8D, 0x0D, 0x20,  // Print status C - should print 00
    0x8D, 0x0E, 0x20,  // Print status Z - should print 01
    0xA0, 0x0B,        // LDY; # 0x0B
    0x8D, 0x0B, 0x20,  // Print Y - should print 0x0B = 11
    0xC0, 0x0A,        // CPY imidiate #0x0A
    0x8D, 0x0C, 0x20,  // Print status N - should print 00
    0x8D, 0x0D, 0x20,  // Print status C - should print 00
    0x8D, 0x0E, 0x20,  // Print status Z - should print 00
    0xA0, 0x02,        // LDY; # 0x02
    0x8D, 0x0B, 0x20,  // Print Y - should print 0x02 = 2
    0xC0, 0x0A,        // CPY imidiate #0x0A
    0x8D, 0x0C, 0x20,  // Print status N - should print 01
    0x8D, 0x0D, 0x20,  // Print status C - should print 01
    0x8D, 0x0E, 0x20,  // Print status Z - should print 00
};

std::vector<uint8_t> cpy_Zeropage{
    0xA9, 0x0A,        // LDA; # 0x0A
    0x8D, 0x09, 0x20,  // Print A - should print 0x0A = 10
    0x8D, 0x20, 0x00,  // reg_a (=0x0A) to 0x0020 in mem
    0xA0, 0x0A,        // LDY; # 0x0A
    0x8D, 0x0B, 0x20,  // Print Y - should print 0x0A = 10
    0xC4, 0x20,        // cpy zeropage mem 0x0020
    0x8D, 0x0C, 0x20,  // Print status N - should print 00
    0x8D, 0x0D, 0x20,  // Print status C - should print 00
    0x8D, 0x0E, 0x20,  // Print status Z - should print 01
    0xA0, 0x0B,        // LDY; # 0x0B
    0x8D, 0x0B, 0x20,  // Print Y - should print 0x0B = 11
    0xC4, 0x20,        // cpy zeropage mem 0x0020
    0x8D, 0x0C, 0x20,  // Print status N - should print 00
    0x8D, 0x0D, 0x20,  // Print status C - should print 00
    0x8D, 0x0E, 0x20,  // Print status Z - should print 00
    0xA0, 0x02,        // LDY; # 0x02
    0x8D, 0x0B, 0x20,  // Print Y - should print 0x02 = 2
    0xC4, 0x20,        // cpy zeropage mem 0x0020
    0x8D, 0x0C, 0x20,  // Print status N - should print 01
    0x8D, 0x0D, 0x20,  // Print status C - should print 00
    0x8D, 0x0E, 0x20,  // Print status Z - should print 00
};

std::vector<uint8_t> cpy_Absolute{
    0xA9, 0x0A,        // LDA; # 0x0A
    0x8D, 0x09, 0x20,  // Print A - should print 0x0A = 10
    0x8D, 0x00, 0x20,  // sta reg_a (=0x0A) to 0x2000 in mem
    0xA0, 0x0A,        // LDY; # 0x0A
    0x8D, 0x0B, 0x20,  // Print Y - should print 0x0A = 10
    0xCC, 0x00, 0x20,  // cpy Absolute mem 0x2000
    0x8D, 0x0C, 0x20,  // Print status N - should print 00
    0x8D, 0x0D, 0x20,  // Print status C - should print 00
    0x8D, 0x0E, 0x20,  // Print status Z - should print 01
    0xA0, 0x0B,        // LDY; # 0x0B
    0x8D, 0x0B, 0x20,  // Print Y - should print 0x0B = 11
    0xCC, 0x00, 0x20,  // cpy Absolute mem 0x2000
    0x8D, 0x0C, 0x20,  // Print status N - should print 00
    0x8D, 0x0D, 0x20,  // Print status C - should print 00
    0x8D, 0x0E, 0x20,  // Print status Z - should print 00
    0xA0, 0x02,        // LDY; # 0x02
    0x8D, 0x0B, 0x20,  // Print Y - should print 0x02 = 2
    0xCC, 0x00, 0x20,  // cpy Absolute mem 0x2000
    0x8D, 0x0C, 0x20,  // Print status N - should print 01
    0x8D, 0x0D, 0x20,  // Print status C - should print 00
    0x8D, 0x0E, 0x20,  // Print status Z - should print 00
};

using namespace llvmes;

int main()
{
    auto d = llvmes::make_unique<Disassembler>(std::move(cmp_AbsoluteY));

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

    bool optimized = true;
    c->GetMain(optimized)();

    return 0;
}
