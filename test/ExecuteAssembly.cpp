#include "llvmes/NES/CPU.h"
#include <string>
#include <iostream>

using namespace llvmes;

std::vector<std::uint8_t> program {
    0xA0, 0x0A, // LDY, # 0x0A
    0xE8,       // INX 
    0x88,       // DEY
    0xD0, 0xFC, // BNE, Begin
    0xEA        // NOP
};

int main()
{
    CPU cpu;
    auto internalRAM = cpu.getMemory();

    cpu.dump();

    std::vector<std::uint8_t> memory(0xFFFF, 0);
    memory[0xFFFC] = 0x20;
    memory[0xFFFD] = 0x40;
    std::copy(program.begin(), program.end(), &memory[0x4020]);

    std::cout << "Address 0x4020: " << std::hex << (unsigned int)memory[0x4020] << std::endl;

    cpu.setExternalMemory(memory);
    for(int i = 0; i < 50; i++) {
        cpu.Execute();
    }
}