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
    int first_int = std::stoi(first, 0, 16);
    int second_int = std::stoi(second, 0, 16);
    return std::make_pair(first_int, second_int);
}

int main(int argc, char** argv)
try {
    cxxopts::Options options("MemView",
                             "Print ranges of memory | compare bin files");

    options.add_options()("f,positional", "File",
                          cxxopts::value<std::string>())(
        "c,compare", "Compare two bin files",
        cxxopts::value<std::vector<std::string>>())(
        "r,range", "Range of memory",
        cxxopts::value<std::vector<std::string>>())("p,print", "Prints memory",
                                                    cxxopts::value<bool>())(
        "h,help",
        "To print a range of memory: memview -p -r 0x0000,0x2000 "
        "file.mem\nNotice no space between arguments and '0x' notation");

    options.parse_positional({"positional"});
    auto result = options.parse(argc, argv);

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
    if (result.count("print")) {
        bool use_range = false;
        std::vector<std::string> range;
        if (result.count("range")) {
            use_range = true;
            range = result["range"].as<std::vector<std::string>>();
        }
        if (use_range && range.size() != 2)
            throw std::runtime_error("Range takes two values!");

        uint16_t start, stop;
        start = 0x0000;
        stop = 0xFFFF;
        if (use_range) {
            auto pair = ParseRange(range[0], range[1]);
            start = pair.first;
            stop = pair.second;
        }
        auto in_file = result["positional"].as<std::string>();
        auto vec = ReadFile(in_file);

        if (stop > vec.size())
            stop = vec.size();

        for (int i = start; i < stop; i++) {
            std::cout << ToHexString(vec.at(i)) << " ";
        }
        std::cout << std::endl;
    }
}
catch (std::exception& e) {
    std::cout << e.what() << std::endl;
}