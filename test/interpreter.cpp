#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

#include "cxxopts.hpp"
#include "llvmes/interpreter/cpu.h"
#include "llvmes/time.h"

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
    // Write status to stdout
    else if (addr == 0x200C) {
        std::cout << ToHexString((uint8_t)cpu->reg_status) << std::endl;
    }
    // Exit program with exit code from reg A
    else if (addr == 0x200F) {
        cpu->Halt();
        // std::cout << "exit: " << (unsigned)cpu->reg_a << std::endl;
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
        "h,help", "Print usage")("t,time", "Set time format (ms/us/s)",
                                 cxxopts::value<std::string>())(
        "s,save", "Save memory to disk", cxxopts::value<std::string>());

    options.parse_positional({"positional"});
    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    bool verbose = false;
    bool save = false;
    TimeFormat time_format = TimeFormat::Micro;

    if (result.count("verbose"))
        verbose = true;
    if (result.count("save"))
        save = true;

    if (result.count("time")) {
        auto t_format = result["time"].as<std::string>();
        if (t_format == "ms")
            time_format = TimeFormat::Milli;
        else if (t_format == "us")
            time_format = TimeFormat::Micro;
        else if (t_format == "s")
            time_format = TimeFormat::Seconds;
    }

    auto input = result["positional"].as<std::string>();
    // End - parsing command line

    // Read in the binary in to host RAM, don't count this into total time
    // Same applies to the JIT Compiler
    std::ifstream in{input, std::ios::binary};
    if (in.fail())
        throw std::runtime_error("The file doesn't exist");
    auto program = std::vector<char>{std::istreambuf_iterator<char>(in),
                                     std::istreambuf_iterator<char>()};

    // Start of Total time is defined as this point, when the actual virtual CPU
    // is created

    ClockType start = high_resolution_clock::now();
    ClockType exec_start, stop;

    cpu = std::make_shared<CPU>();

    std::copy(program.begin(), program.end(), &memory[0x8000]);

    cpu->Read = readMemory;
    cpu->Write = writeMemory;
    cpu->Reset();

    exec_start = high_resolution_clock::now();
    cpu->Run();
    stop = high_resolution_clock::now();

    if (verbose) {
        std::cout << "Execution time: "
                  << GetDuration<ClockType>(time_format, exec_start, stop)
                  << GetTimeFormatAbbreviation(time_format) << std::endl;
        std::cout << "Total time: "
                  << GetDuration<ClockType>(time_format, start, stop)
                  << GetTimeFormatAbbreviation(time_format) << std::endl;
    }

    if (save) {
        std::string out = result["save"].as<std::string>();
        std::stringstream ss;
        ss << input << ".mem";
        if (out.empty())
            out = ss.str();
        auto fstream = std::fstream(out, std::ios::out | std::ios::binary);
        fstream.write((char*)memory.data(), memory.size());
        fstream.close();
    }

    return 0;
}
catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
}
