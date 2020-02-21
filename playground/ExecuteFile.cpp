#include "llvmes/NES/CPU.h"
#include "llvmes/NES/program_loader.h"
#include <string>
#include <iostream>
#include <memory>

using namespace llvmes;

std::vector<std::uint8_t> memory(0xFFFF);

std::uint8_t readMemory(std::uint16_t adr) {
    return memory[adr];
}

void writeMemory(std::uint16_t adr, std::uint8_t data) {
    memory[adr] = data;
}

int main(int argc, char **argv) try {
    auto programLoader = std::make_unique<ProgramLoader>("xd.o");

    std::vector<uint8_t> program = programLoader->GetProgram();

    // Load program into virtual computer memory
    memory[0xFFFC] = 0x20;
    memory[0xFFFD] = 0x40;
    std::copy(program.begin(), program.end() - 1, &memory[0x4020]);

    CPU cpu;
    cpu.read = readMemory;
    cpu.write = writeMemory;
    cpu.reset();

    cpu.run();

    return 0;

} catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
}
