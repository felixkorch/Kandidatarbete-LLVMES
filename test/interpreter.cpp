#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

#include "cxxopts.hpp"
#include "llvmes/interpreter/cpu.h"

using namespace llvmes;
using namespace std::chrono;

std::vector<std::uint8_t> memory(0xFFFF);

std::shared_ptr<CPU> cpu;

std::uint8_t readMemory(std::uint16_t adr)
{
    return memory[adr];
}

void writeMemory(std::uint16_t addr, std::uint8_t data)
{
    // Write to '0x2008' and 'A' will be written to stdout as char
    if (addr == 0x2008) {
        std::cout << cpu->reg_a;
    }
    // Write A to stdout
    else if (addr == 0x2009) {
        std::cout << ToHexString(cpu->reg_a) << std::endl;
    }
    // Write X to stdout
    else if (addr == 0x200A) {
        std::cout << ToHexString(cpu->reg_x) << std::endl;
    }
    // Write Y to stdout
    else if (addr == 0x200B) {
        std::cout << ToHexString(cpu->reg_y) << std::endl;
    }
    // Write N to stdout
    else if (addr == 0x200C) {
        std::cout << cpu->reg_status.N << std::endl;
    }
    // Write C to stdout
    else if (addr == 0x200D) {
        std::cout << cpu->reg_status.C << std::endl;
    }
    // Write Z to stdout
    else if (addr == 0x200E) {
        std::cout << cpu->reg_status.Z << std::endl;
    }
    // Exit program with exit code from reg A
    else if (addr == 0x200F) {
        cpu->Halt();
        std::cout << "exit: " << (unsigned)cpu->reg_a << std::endl;
    }
    else {
        memory[addr] = data;
    }
}

int main(int argc, char** argv)
try {
    cxxopts::Options options("Interpreter",
                             "Run a bin file using the interpreter");

    options.add_options()("f,positional", "File",
                          cxxopts::value<std::string>())(
        "v,verbose", "Enable verbose output", cxxopts::value<bool>())(
        "h,help", "Print usage");

    options.parse_positional({"positional"});
    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    bool verbose = false;

    if (result.count("verbose"))
        verbose = true;

    auto input = result["positional"].as<std::string>();

    auto start = high_resolution_clock::now();

    cpu = std::make_shared<CPU>();

    std::ifstream in{input, std::ios::binary};
    if (in.fail())
        throw std::runtime_error("The file doesn't exist");
    auto program = std::vector<char>{std::istreambuf_iterator<char>(in),
                                     std::istreambuf_iterator<char>()};

    std::copy(program.begin(), program.end(), &memory[0x8000]);

    cpu->Read = readMemory;
    cpu->Write = writeMemory;
    cpu->Reset();

    auto exec_start = high_resolution_clock::now();
    cpu->Run();

    auto stop = high_resolution_clock::now();

    auto exec_time = duration_cast<microseconds>(stop - exec_start);
    auto total_time = duration_cast<microseconds>(stop - start);

    std::cout << "Execution time: " << exec_time.count() << "us" << std::endl;
    std::cout << "Total time: " << total_time.count() << "us" << std::endl;

    return 0;
}
catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
}
