#include <chrono>
#include <fstream>

#include "cxxopts.hpp"
#include "llvmes/dynarec/compiler.h"
#include "llvmes/dynarec/disassembler.h"

using namespace llvmes;
using namespace std::chrono;

enum class TimeFormat { Milli, Micro, Seconds };

long long GetDuration(TimeFormat format, steady_clock::time_point start,
                      steady_clock::time_point stop)
{
    switch (format) {
        case TimeFormat::Micro:
            return duration_cast<microseconds>(stop - start).count();
        case TimeFormat::Milli:
            return duration_cast<milliseconds>(stop - start).count();
        case TimeFormat::Seconds:
            return duration_cast<seconds>(stop - start).count();
        default:
            return duration_cast<microseconds>(stop - start).count();
    }
}

std::string GetTimeFormatAbbreviation(TimeFormat format)
{
    switch (format) {
        case TimeFormat::Micro:
            return "us";
        case TimeFormat::Milli:
            return "ms";
        case TimeFormat::Seconds:
            return "s";
        default:
            return "us";
    }
}

int main(int argc, char** argv)
try {
    cxxopts::Options options("JIT Compiler",
                             "Run a bin file using the jit compiler");

    options.add_options()("f,positional", "File",
                          cxxopts::value<std::string>())(
        "v,verbose", "Write IR to file", cxxopts::value<bool>())(
        "O,optimize", "Optimize", cxxopts::value<bool>())(
        "h,help", "Print usage")("t,time", "Set time format (ms/us/s)",
                                 cxxopts::value<std::string>())(
        "s,save", "Save memory to disk", cxxopts::value<bool>());

    options.parse_positional({"positional"});
    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    TimeFormat time_format = TimeFormat::Micro;
    bool verbose, optimize, save;
    verbose = optimize = save = false;

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

    if (result.count("verbose"))
        verbose = true;

    if (result.count("optimize"))
        optimize = true;

    if (result.count("save"))
        save = true;

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

    std::cout << "Execution time: "
              << GetDuration(time_format, exec_start, stop)
              << GetTimeFormatAbbreviation(time_format) << std::endl;
    std::cout << "Total time: " << GetDuration(time_format, start, stop)
              << GetTimeFormatAbbreviation(time_format) << std::endl;

    if (save) {
        auto ram_ref = c->GetRAMRef();
        std::stringstream ss;
        ss << input << ".mem";
        auto fstream = std::fstream(ss.str(), std::ios::out | std::ios::binary);
        fstream.write((char*)ram_ref.data(), ram_ref.size());
        fstream.close();
    }

    return 0;
}
catch (std::exception& e) {
    std::cout << e.what() << std::endl;
}
