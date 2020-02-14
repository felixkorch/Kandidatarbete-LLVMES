#include "llvmes/NES/CPU.h"
#include <string>

using namespace llvmes;

std::vector<std::uint8_t> program {
    0xA0, 0x0A, // LDY, # 0x0A
    0xE8,       // INX 
    0x88,       // DEY
    0xD0, 0xFC, // BNE, Begin
    0xEA        // NOP
};

std::vector<std::uint8_t> memory(0xFFFF, 0);

std::uint8_t readMemory(std::uint16_t adr)
{
    return memory[adr];
}

void writeMemory(std::uint16_t adr, std::uint8_t data)
{
    memory[adr] = data;
}

int main()
{
    memory[0xFFFC] = 0x20;
    memory[0xFFFD] = 0x40;
    std::copy(program.begin(), program.end(), &memory[0x4020]);

    CPU cpu;
    cpu.read = readMemory;
    cpu.write = writeMemory;
    cpu.reset();
    cpu.dump();

    cpu.run();
}