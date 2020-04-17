#include <fstream>
#include <iostream>
#include <string>

#include "llvmes/interpreter/cpu.h"

using namespace llvmes;

std::vector<std::uint8_t> memory(0xFFFF);

CPU* s_cpu = nullptr;

std::uint8_t readMemory(std::uint16_t adr)
{
    return memory[adr];
}

void writeMemory(std::uint16_t addr, std::uint8_t data)
{
    // Write to '0x2008' and 'A' will be written to stdout as char
    if (addr == 0x2008) {
        std::cout << s_cpu->reg_a;
    }
    // Write A to stdout
    else if (addr == 0x2009) {
        std::cout << ToHexString(s_cpu->reg_a) << std::endl;
    }
    // Write X to stdout
    else if (addr == 0x200A) {
        std::cout << ToHexString(s_cpu->reg_x) << std::endl;
    }
    // Write Y to stdout
    else if (addr == 0x200B) {
        std::cout << ToHexString(s_cpu->reg_y) << std::endl;
    }
    // Write N to stdout
    else if (addr == 0x200C) {
        std::cout << s_cpu->reg_status.N << std::endl;
    }
    // Write C to stdout
    else if (addr == 0x200D) {
        std::cout << s_cpu->reg_status.C << std::endl;
    }
    // Write Z to stdout
    else if (addr == 0x200E) {
        std::cout << s_cpu->reg_status.Z << std::endl;
    }
    // Exit program with exit code from reg A
    else if (addr == 0x200F) {
        s_cpu->Halt();
        std::cout << "exit: " << (unsigned)s_cpu->reg_a << std::endl;
    }
    else {
        memory[addr] = data;
    }
}

int main(int argc, char** argv)
try {
    if (argc == 1) {
        std::cout << "No path provided." << std::endl;
        return 1;
    }
    else if (argc > 2) {
        std::cout << "Only one argument allowed." << std::endl;
        return 1;
    }

    std::ifstream in{argv[1], std::ios::binary};
    if (in.fail())
        throw std::runtime_error("The file doesn't exist");
    auto program = std::vector<char>{std::istreambuf_iterator<char>(in),
                                     std::istreambuf_iterator<char>()};

    std::copy(program.begin(), program.end(), &memory[0x8000]);

    CPU cpu;
    s_cpu = &cpu;
    cpu.Read = readMemory;
    cpu.Write = writeMemory;
    cpu.Reset();

    cpu.Run();

    return 0;
}
catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
}
