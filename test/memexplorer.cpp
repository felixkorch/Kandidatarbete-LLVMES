#include <fstream>
#include <utility>

#include "cxxopts.hpp"
#include "llvmes/common.h"

std::vector<uint8_t> ReadFile(const std::string& filepath)
{
    std::ifstream in{filepath, std::ios::binary};
    if (in.fail())
        throw std::runtime_error("The file doesn't exist");
    auto in_file = std::vector<uint8_t>{std::istreambuf_iterator<char>(in),
                                        std::istreambuf_iterator<char>()};
    return std::move(in_file);
}

bool CompareBins(const std::string& first, const std::string& second)
{
    auto bin1 = ReadFile(first);
    auto bin2 = ReadFile(second);
    bool equal = true;
    for (int i = 0; i < bin1.size(); i++) {
        if (bin1[i] != bin2[i])
            equal = false;
    }
    return equal;
}

using namespace llvmes;

std::pair<int, int> ParseRange(const std::string& first,
                               const std::string& second)
{
    int first_int = HexStringToInt(first);
    int second_int = HexStringToInt(second);
    return std::make_pair(first_int, second_int);
}

int main(int argc, char** argv)
try {
    cxxopts::Options options("Mem Explorer",
                             "Print ranges of memory | compare bin files");

    options.add_options()("f,positional", "File",
                          cxxopts::value<std::string>())(
        "c,compare", "Compare two bin files",
        cxxopts::value<std::vector<std::string>>())(
        "r,range", "Range of memory",
        cxxopts::value<std::vector<std::string>>())(
        "p,print", "Print?", cxxopts::value<bool>())("h,help", "Print usage");

    auto result = options.parse(argc, argv);
    options.parse_positional({"positional"});

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    if (result.count("compare")) {
        auto input = result["compare"].as<std::vector<std::string>>();
        if (input.size() != 2)
            throw std::runtime_error("Compare takes two inputs!");
        bool eq = CompareBins(input[0], input[1]);
        std::cout << (eq ? "Equal" : "Not equal") << std::endl;
    }
    else if (result.count("print")) {
        int range_count = result.count("range");
        if (range_count && range_count != 2)
            throw std::runtime_error("Range takes two values!");

        uint16_t start, stop;
        start = 0x0000;
        stop = 0xFFFF;
        if (range_count) {
            auto range = result["range"].as<std::vector<std::string>>();
            auto pair = ParseRange(range[0], range[1]);
            start = pair.first;
            stop = pair.second;
        }

        auto in_file = result["positional"].as<std::string>();
        auto vec = ReadFile(in_file);

        for (int i = start; i < stop; i++) {
            std::cout << ToHexString(vec.at(i)) << " ";
        }
    }
}
catch (std::exception& e) {
    std::cout << e.what() << std::endl;
}