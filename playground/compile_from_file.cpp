#include "llvmes/dynarec/compiler.h"
#include "llvmes/dynarec/disassembler.h"

#include <fstream>

using namespace llvmes;

int main(int argc, char** argv)
{
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
    auto in_file = std::vector<uint8_t>{std::istreambuf_iterator<char>(in),
                                     std::istreambuf_iterator<char>()};

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

    auto c = llvmes::make_unique<Compiler>(std::move(ast), argv[1]);
    c->SetRAM(std::move(ram));
    c->Compile();

    bool optimized = true;
    c->GetMain(optimized)();

    return 0;
}
