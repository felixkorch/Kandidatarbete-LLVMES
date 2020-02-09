#include "llvmes/NES/CPU.h"
#include <string>
#include <functional>

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

/*
class NESInstance {
private:
    CPU cpu;
    CPU::BusRead read;
    CPU::BusWrite write;
    std::vector<std::uint8_t> mem;
public:

    NESInstance()
    : cpu(read, write)
    {
        read = std::bind(&NESInstance::readMemory, this, std::placeholders::_1);
        write = std::bind(&NESInstance::readMemory, this, std::placeholders::_2);
    }

    std::uint8_t readMemory(std::uint16_t address)
    {
        if(address <= 0x1FFF) {
            // %0x800 is modulus. in the scope of 0x0000 - 0x1fff is one RAM with 3 mirroring
            return mem[address % 0x800];
        }
        if(address <= 0x3FFF) {
            // TODO: PPU
        }
        if(address <= 0x4017) {
            // TODO: APU / IO
        }
        if(address <= 0x401F) {
            // DISABLED
        }
        return mem[address];
    }

    void writeMemory(std::uint16_t address, std::uint8_t)
    {

    }
};
*/

int main()
{
    memory[0xFFFC] = 0x20;
    memory[0xFFFD] = 0x40;
    std::copy(program.begin(), program.end(), &memory[0x4020]);

    CPU cpu(readMemory, writeMemory);
    cpu.reset();
    cpu.dump();

    for(int i = 0; i < 50; i++) {
        cpu.execute();
    }
}