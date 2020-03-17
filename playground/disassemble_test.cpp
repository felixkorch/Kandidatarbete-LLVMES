#include "llvmes/disassembler.h"

std::vector<std::uint8_t> program {
        0xA0, 0x0A,   // LDY, # 0x0A
        0xE8,         // INX -- Begin
        0x88,         // DEY
        0xD0, 0xFD,   // BNE, Begin
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

int main()
{
    Disassembler dis(std::move(program));
    dis.Disassemble();
    dis.PrintAST();
}