#include "llvmes/interpreter/cpu.h"
#include <string>
#include <fstream>
#include <iostream>

using namespace llvmes;

std::vector<std::uint8_t> memory(0xFFFF);

std::uint8_t readMemory(std::uint16_t adr)
{
    return memory[adr];
}

void writeMemory(std::uint16_t adr, std::uint8_t data)
{
    memory[adr] = data;
}

int main(int argc, char** argv) try
{
    if (argc == 1) {
        std::cout << "No path provided." << std::endl;
        return 1;
    }
    else if (argc > 2) {
        std::cout << "Only one argument allowed." << std::endl;
        return 1;
    }

    std::ifstream in{ argv[1], std::ios::binary };
    if (in.fail())
        throw std::runtime_error("The file doesn't exist");
    auto program = std::vector<char>{ std::istreambuf_iterator<char>(in),
            std::istreambuf_iterator<char>() };

    memory[0xFFFC] = 0x20;
    memory[0xFFFD] = 0x40;
    std::copy(program.begin(), program.end(), &memory[0x4020]);

    CPU cpu;
    cpu.Read = readMemory;
    cpu.Write = writeMemory;
    cpu.Reset();

    cpu.Run();

    return 0;

} catch(std::exception& e){
        std::cerr << e.what() << std::endl;
}
