#include "llvmes/dynarec/compiler.h"
#include "llvmes/dynarec/disassembler.h"

#define PRINT_A 0x8D, 0x09, 0x20
#define PRINT_X 0x4C, 0x14, 0x00
#define PRINT_Y 0x8D, 0x0B, 0x20
#define PRINT_N 0x8D, 0x0C, 0x20
#define PRINT_C 0x8D, 0x0D, 0x20
#define PRINT_Z 0x8D, 0x0E, 0x20
#define PRINT_V 0x8D, 0x0F, 0x20
#define LDY_IMM(V) 0xA0, V
#define LDX_IMM(V) 0xA2, V
#define INX 0xE8
#define DEY 0x88
#define STX_ZPG_Y(V) 0x96, V
#define STY_ZPG(V) 0x84, V
#define STY_ABS(B1, B2) 0x8C, B2, B1
#define STX_ABS(B1, B2) 0x8E, B2, B1
#define REL(X) 0xFE X
#define LDA_IMM(V) 0xA9, V
#define LDA_ABS(B1, B2) 0xAD, B2, B1
#define STA_ABS(B1, B2) 0x8D, B2, B1
#define BNE(V) 0xD0, REL(V)
#define BEQ(V) 0xF0, REL(V)
#define BMI(V) 0x30, REL(V)
#define BCC(V) 0x90, REL(V)
#define BCS(V) 0xB0, REL(V)
#define BPL(V) 0x10, REL(V)
#define BVC(V) 0x50, REL(V)
#define BVS(V) 0x70, REL(V)
#define BIT_ZPG(V) 0x24, V
#define BIT_ABS(B1, B2) 0x2C, B2, B1

std::vector<uint8_t> program1{
    0xA0, 0x0A,        // LDY, # 0x0A
    0xE8,              // INX -- Begin
    0x88,              // DEY
    0x8D, 0x0A, 0x20,  // Print X
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

std::vector<uint8_t> testINC_zeropageX{
    0xA9, 0x0A,  // LDA; # 0x0A
    0x8D, 0x09, 0x20,  // Print A - should print 0x0A = 10
    0x8D, 0x21, 0x00,  // reg_a (=0x0A) to 0x0020 in mem
    LDX_IMM(0x01),
    0xF6, 0x20,  // inc zeropage mem 0x0020 + x
    0xB5, 0x20,
    0x8D, 0x09, 0x20,  // Print A - should print 0x0A = 10
    0x8D, 0x0C, 0x20,  // Print status N - should print 00
    0x8D, 0x0E, 0x20,  // Print status Z - should print 01
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


std::vector<uint8_t> ror_Accumulator{
    // ROR C=0 in
    0xA9, 0xFF,        // LDA; # 0x01
    0x8D, 0x09, 0x20,  // Print A - should print 0xFF
    0x6A,              // ROR; (accumulator)
    0x8D, 0x09, 0x20,  // Print A - should print 0x7F
    // CMP imidiate to set C
    0xA9, 0x02,        // LDA; # 0x02
    0xC9, 0x0A,        // CMP imidiate #0x0A
    // ROR C=1 in
    0xA9, 0xFF,        // LDA; # 0x01
    0x8D, 0x09, 0x20,  // Print A - should print 0xFF
    0x6A,              // ROR; (accumulator)
    0x8D, 0x09, 0x20,  // Print A - should print 0xFF
};

std::vector<uint8_t> ror_Zeropage{
    // ROR C=0 in
    0xA9, 0xFF,        // LDA; # 0xFF
    0x8D, 0x09, 0x20,  // Print A - should print 0xFF
    0x8D, 0x20, 0x00,  // reg_a (=0xFF) to 0x0020 in mem
    0x66, 0x20,        // ROR: (zeropage) # 0x20
    0x8D, 0x09, 0x20,  // Print A - should print 0x7F
    // CMP imidiate to set C
    0xA9, 0x02,        // LDA; # 0x02
    0xC9, 0x0A,        // CMP imidiate #0x0A
    // ROR C=1 in
    0xA9, 0xFF,        // LDA; # 0xFF
    0x8D, 0x09, 0x20,  // Print A - should print 0xFF
    0x66, 0x20,        // ROR; (zeropage) # 0x20
    0x8D, 0x09, 0x20,  // Print A - should print 0xFF
};

std::vector<uint8_t> ror_ZeropageX{
    // ROR C=0 in
    0xA9, 0xFF,        // LDA; # 0xFF
    0x8D, 0x09, 0x20,  // Print A - should print 0xFF
    0x8D, 0x22, 0x00,  // reg_a (=0xFF) to 0x0022 in mem
    0xA2, 0x02,        // LDX; # 0x02
    0x8D, 0x0A, 0x20,  // Print X - should print 0x02
    0x76, 0x20,        // ROR; (zeropagX) # 0x20
    0x8D, 0x09, 0x20,  // Print A - should print 0x7F
    // CMP imidiate to set C
    0xA9, 0x02,  // LDA; # 0x02
    0xC9, 0x0A,  // CMP imidiate #0x0A
    // ROR C=1 in
    0xA9, 0xFF,        // LDA; # 0xFF
    0x8D, 0x09, 0x20,  // Print A - should print 0xFF
    0x76, 0x20,        // ROR; (zeropagX) # 0x20
    0x8D, 0x09, 0x20,  // Print A - should print 0xFF
};

std::vector<uint8_t> ror_Absolute{
    // ROR C=0 in
    0xA9, 0xFF,        // LDA; # 0xFF
    0x8D, 0x09, 0x20,  // Print A - should print 0xFF
    0x8D, 0x00, 0x20,  // reg_a (=0xFF) to 0x2000 in mem
    0x6E, 0x00, 0x20,  // ROR (Absolute) # 0x2000
    0x8D, 0x09, 0x20,  // Print A - should print 0x7F
    // CMP imidiate to set C
    0xA9, 0x02,  // LDA; # 0x02
    0xC9, 0x0A,  // CMP imidiate #0x0A
    // ROR C=1 in
    0xA9, 0xFF,        // LDA; # 0xFF
    0x8D, 0x09, 0x20,  // Print A - should print 0xFF
    0x6E, 0x00, 0x20,  // ROR (Absolute)  # 0x2000
    0x8D, 0x09, 0x20,  // Print A - should print 0xFF
};

std::vector<uint8_t> ror_AbsoluteX{
    // ROR C=0 in
    0xA9, 0xFF,        // LDA; # 0xFF
    0x8D, 0x09, 0x20,  // Print A - should print 0xFF
    0x8D, 0x02, 0x20,  // reg_a (=0xFF) to 0x2002 in mem
    0xA2, 0x02,        // LDX; # 0x02
    0x8D, 0x0A, 0x20,  // Print X - should print 0x02
    0x7E, 0x00, 0x20,  // ROR (AbsoluteX) # 2000
    0x8D, 0x09, 0x20,  // Print A - should print 0x7F
    // CMP imidiate to set C
    0xA9, 0x02,  // LDA; # 0x02
    0xC9, 0x0A,  // CMP imidiate #0x0A
    // ROR C=1 in
    0xA9, 0xFF,        // LDA; # 0xFF
    0x8D, 0x09, 0x20,  // Print A - should print 0xFF
    0x7E, 0x00, 0x20,  // ROR (AbsoluteX) # 2000
    0x8D, 0x09, 0x20,  // Print A - should print 0xFF
    
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
    0x8D, 0x0D, 0x20,  // Print status C - should print 01
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
    0x8D, 0x0D, 0x20,  // Print status C - should print 01
    0x8D, 0x0E, 0x20,  // Print status Z - should print 00
};

std::vector<uint8_t> SBC_Absolute{
    0xA9, 0x05,        // LDA; # 0x05
    0x8D, 0x09, 0x20,  // Print A - should print 0x05 = 5
    0x8D, 0x00, 0x20,  // sta reg_a (=0x05) to 0x2000 in mem
    0xA9, 0x0A,        // LDA; # 0x0A
    0x8D, 0x09, 0x20,  // Print A - should print 0x0A = 10
    0xED, 0x00, 0x20,  // SBC Immediate
    0x8D, 0x09, 0x20,  // Print A - should print 0x05 = 5
    0x8D, 0x0C, 0x20,  // Print status N - should print 00
    0x8D, 0x0D, 0x20,  // Print status C - should print 00
    0x8D, 0x0E, 0x20,  // Print status Z - should print 00
    0xED, 0x00, 0x20,  // SBC Immediate
    0x8D, 0x09, 0x20,  // Print A - should print 0x05 = -2
    0x8D, 0x0C, 0x20,  // Print status N - should print 01

    //0x8D, 0x0C, 0x20,  // Print status N - should print 00
    //0x8D, 0x0D, 0x20,  // Print status C - should print 00
    //0x8D, 0x0E, 0x20,  // Print status Z - should print 01
    //0xA0, 0x0B,        // LDY; # 0x0B
    //0x8D, 0x0B, 0x20,  // Print Y - should print 0x0B = 11
    //0xCC, 0x00, 0x20,  // cpy Absolute mem 0x2000
    //0x8D, 0x0C, 0x20,  // Print status N - should print 00
    //0x8D, 0x0D, 0x20,  // Print status C - should print 00
    //0x8D, 0x0E, 0x20,  // Print status Z - should print 00
    //0xA0, 0x02,        // LDY; # 0x02
    //0x8D, 0x0B, 0x20,  // Print Y - should print 0x02 = 2
    //0xCC, 0x00, 0x20,  // cpy Absolute mem 0x2000
    //0x8D, 0x0C, 0x20,  // Print status N - should print 01
    //0x8D, 0x0D, 0x20,  // Print status C - should print 01
    //0x8D, 0x0E, 0x20,  // Print status Z - should print 00
};

std::vector<uint8_t> ora_Absolute
{
    LDA_IMM(0x0A),     // LDA #0x0A
    PRINT_A,           // Print A - should print 0x0A = 10
    LDX_IMM(0xA0),     // LDX #0xA0
    PRINT_X,           // Print X - should print 0xA0 = 160
    STX_ABS(0x12, 0x34),// STX $1234
    0x0D, 0x34, 0x12,  // ORA $1234
    PRINT_A,           // Print A - should print 0xAA = 170
};

std::vector<uint8_t> ora_Immediate
{
    LDA_IMM(0x0A),     // LDA #0x0A
    PRINT_A,           // Print A - should print 0x0A = 10
    0x09, 0xA0,        // ORA #0xA0
    PRINT_A,           // Print A - should print 0xAA = 170
};

std::vector<uint8_t> testBIT{
    // Test BIT_ZPG
    LDA_IMM(0xFF), STA_ABS(0x00, 0x00), LDA_IMM(0xFF), BIT_ZPG(0x00),
    PRINT_N,  // should print 1
    PRINT_V,  // should print 1
    PRINT_Z,  // should print 0
    LDA_IMM(0x01), STA_ABS(0x00, 0x00), LDA_IMM(0x00), BIT_ZPG(0x00),
    PRINT_N,  // should print 0
    PRINT_V,  // should print 0
    PRINT_Z,  // should print 1
    // Test BIT_ABS
    LDA_IMM(0xFF), STA_ABS(0xF0, 0xF0), LDA_IMM(0xFF), BIT_ABS(0xF0, 0xF0),
    PRINT_N,  // should print 1
    PRINT_V,  // should print 1
    PRINT_Z,  // should print 0
    LDA_IMM(0x01), STA_ABS(0xF0, 0xF0), LDA_IMM(0x00), BIT_ABS(0xF0, 0xF0),
    PRINT_N,  // should print 0
    PRINT_V,  // should print 0
    PRINT_Z,  // should print 1
};

std::vector<uint8_t> txs_Implied{
    0xA2, 0x0A,        // LDX, # 0x0A
    0x8D, 0x0A, 0x20,  // Print X - should be 0x0A = 10
    //0x8D, 0x0F, 0x20,  // Print Sp - should be 0x?? = ??
    0x9A,              // TXS Imidiate (9A)
    //0x8D, 0x0F, 0x20,  // Print Sp - should be 0x0A = 10
};

std::vector<uint8_t> tya_Implied{
    0xA9, 0x00,        // LDA, # 0x00  
    0xA0, 0x0A,        // LDY, # 0x0A
    0x8D, 0x09, 0x20,  // Print A - should be 0x00 = 0
    0x8D, 0x0B, 0x20,  // Print Y - should be 0x0A = 10  
    0x98,              // TYA implied
    0x8D, 0x0B, 0x20,   // Print Y - should be 0x0A = 10
    0x8D, 0x09, 0x20,  // Print A - should be 0x0A = 10
    0x8D, 0x0C, 0x20,  // Print N - should be 0x00 = 0
    0x8D, 0x0E, 0x20,  // Print Z - should be 0x00 = 0
    0xA9, 0x00,        // LDA, # 0x00
    0xA0, 0xFF,        // LDY, # 0xFF
    0x8D, 0x09, 0x20,  // Print A - should be 0x00 = 0
    0x8D, 0x0B, 0x20,  // Print Y - should be 0xFF = 255
    0x98,              // TYA implied
    0x8D, 0x0B, 0x20,  // Print Y - should be 0xFF = 255
    0x8D, 0x09, 0x20,  // Print A - should be 0xFF = 255
    0x8D, 0x0C, 0x20,  // Print N - should be 0x00 = 1
    0x8D, 0x0E, 0x20,  // Print Z - should be 0x00 = 0
    0xA9, 0x0A,        // LDA, # 0x0A
    0xA0, 0x00,        // LDY, # 0x00
    0x8D, 0x09, 0x20,  // Print A - should be 0x0A = 10
    0x8D, 0x0B, 0x20,  // Print Y - should be 0x00 = 0
    0x98,              // TYA implied
    0x8D, 0x0B, 0x20,  // Print Y - should be 0x00 = 0
    0x8D, 0x09, 0x20,  // Print A - should be 0x00 = 0
    0x8D, 0x0C, 0x20,  // Print N - should be 0x00 = 0
    0x8D, 0x0E, 0x20,  // Print Z - should be 0x00 = 1
};

std::vector<uint8_t> txa_Implied{
    // First test 0x0A to A
    0xA9, 0x00,        // LDA, # 0x00
    0xA2, 0x0A,        // LDX, # 0x0A
    0x8D, 0x09, 0x20,  // Print A - should be 0x00 = 0
    0x8D, 0x0A, 0x20,  // Print X - should be 0x0A = 10  
    0x8A,              // TXA implied
    0x8D, 0x0A, 0x20,  // Print X - should be 0x0A = 10
    0x8D, 0x09, 0x20,  // Print A - should be 0x0A = 10
    0x8D, 0x0C, 0x20,  // Print N - should be 0x00 = 0
    0x8D, 0x0E, 0x20,  // Print Z - should be 0x00 = 0
    // Second test 0xFF to A
    0xA9, 0x00,        // LDA, # 0x00
    0xA2, 0xFF,        // LDX, # 0xFF
    0x8D, 0x09, 0x20,  // Print A - should be 0x00 = 0
    0x8D, 0x0A, 0x20,  // Print X - should be 0xFF = 255
    0x8A,              // TXA implied
    0x8D, 0x0A, 0x20,  // Print X - should be 0xFF = 255
    0x8D, 0x09, 0x20,  // Print A - should be 0xFF = 255
    0x8D, 0x0C, 0x20,  // Print N - should be 0x00 = 1
    0x8D, 0x0E, 0x20,  // Print Z - should be 0x00 = 0
    // Third test 0x00 to A
    0xA9, 0x0A,        // LDA, # 0x0A
    0xA2, 0x00,        // LDX, # 0x00
    0x8D, 0x09, 0x20,  // Print A - should be 0x0A = 10
    0x8D, 0x0A, 0x20,  // Print X - should be 0x00 = 0
    0x8A,              // TXA implied
    0x8D, 0x0A, 0x20,  // Print X - should be 0x00 = 0
    0x8D, 0x09, 0x20,  // Print A - should be 0x00 = 0
    0x8D, 0x0C, 0x20,  // Print N - should be 0x00 = 0
    0x8D, 0x0E, 0x20,  // Print Z - should be 0x00 = 1

};

using namespace llvmes;

int main()
{
    auto d = llvmes::make_unique<Disassembler>(std::move(testINC_zeropageX), 0x8000);
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


    ast.Print();

    auto c = llvmes::make_unique<Compiler>(std::move(ast), "load_store");
    c->SetRAM(std::move(ram));
    c->Compile();

    bool optimized = false;
    c->GetMain(optimized)();

    return 0;
}
