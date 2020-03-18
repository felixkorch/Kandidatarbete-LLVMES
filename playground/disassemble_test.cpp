#include "llvmes/disassembler.h"

std::vector<uint8_t> program1 {
        0xA0, 0x0A,   // LDY, # 0x0A
        0xE8,         // INX -- Begin
        0x88,         // DEY
        0xD0, 0xFC,   // BNE, Begin
        0xEA,         // NOP
        0xEA,         // NOP
        0xEA,         // NOP
        0xEA,         // NOP
        0xEA,         // NOP
        0x4C, 0x10, 0x00, // JMP to 'dest'
        0xEA,         // NOP
        0xEA,         // NOP
        0xEA,         // NOP -- dest
        0xEA,         // NOP
        0xEA,         // NOP
};

std::vector<uint8_t> program2 {
        0xa2,
        0x08,
        0xca,
        0x8e,
        0x00,
        0x02,
        0xe0,
        0x03,
        0xd0,
        0xf8,
        0x8e,
        0x01,
        0x02,
        0x00
};

std::vector<uint8_t> program3 {
        0xa9,
        0x01,
        0xc9,
        0x02,
        0xd0,
        0x02,
        0x85,
        0x22,
        0x00
};
int main()
{
    Disassembler dis(std::move(program1));
    dis.Disassemble();
    dis.PrintAST();
}