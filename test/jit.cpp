#include <chrono>
#include <fstream>

#include "cxxopts.hpp"
#include "llvmes/dynarec/compiler.h"
#include "llvmes/dynarec/disassembler.h"

using namespace llvmes;
using namespace std::chrono;

int main(int argc, char** argv)
try {
    cxxopts::Options options("JIT Compiler",
                             "Run a bin file using the jit compiler");

    options.add_options()("f,positional", "File",
                          cxxopts::value<std::string>())(
        "v,verbose", "Write IR to file", cxxopts::value<bool>())(
        "O,optimize", "Optimize", cxxopts::value<bool>())("h,help",
                                                          "Print usage");

    options.parse_positional({"positional"});
    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    auto input = result["positional"].as<std::string>();

    bool verbose, optimize;
    verbose = optimize = false;

    if (result.count("verbose"))
        verbose = true;

    if (result.count("optimize"))
        optimize = true;

    std::ifstream in{input, std::ios::binary};
    if (in.fail())
        throw std::runtime_error("The file doesn't exist");
    auto in_file = std::vector<uint8_t>{std::istreambuf_iterator<char>(in),
                                        std::istreambuf_iterator<char>()};

    auto start = high_resolution_clock::now();

    auto d = llvmes::make_unique<Disassembler>(std::move(in_file), 0x8000);

    AST ast;
    std::vector<uint8_t> ram;
    try {
        ast = d->Disassemble();
        ram = d->GetRAM();
    }
    catch (ParseException& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    auto c = llvmes::make_unique<Compiler>(std::move(ast), input);
    if (verbose)
        c->SetDumpDir(".");
    c->SetRAM(std::move(ram));
    c->Compile();

    auto main = c->GetMain(optimize);

    auto exec_start = high_resolution_clock::now();
    main();
    auto stop = high_resolution_clock::now();

    auto exec_time = duration_cast<microseconds>(stop - exec_start);
    auto total_time = duration_cast<microseconds>(stop - start);

    std::cout << "Execution time: " << exec_time.count() << "us" << std::endl;
    std::cout << "Total time: " << total_time.count() << "us" << std::endl;

    return 0;
}
catch (std::exception& e) {
    std::cout << e.what() << std::endl;
}
