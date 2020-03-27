#include "llvmes/dynarec/codegen.h"
#include "llvmes/dynarec/disassembler.h"

std::vector<uint8_t> program1{
    0xA0, 0x0A,        // LDY, # 0x0A
    0xE8,              // INX -- Begin
    0x88,              // DEY
    0x8D, 0x0A, 0x20,   // Print X
    0xD0, 0xF9,        // BNE, Begin
};

std::vector<uint8_t> program2{
    0xA0, 0x0A,        // LDY, # 0x0A
    0xE8,              // INX -- Begin
    0x88,              // DEY
    0xD0, 0xFC,        // BNE, Begin
    0x8E, 0x00, 0x00,  // STX, $0000
    0xAD, 0x00, 0x00,  // LDA, $0000
    0x8D, 0x09, 0x20,  // Print A - should print 10
    0x4C, 0x14, 0x00,  // JMP to Print X
    0xEA,
    0xEA,
    0x8D, 0x0A, 0x20   // Print X
};

using namespace llvmes;

int main()
{
    auto d = llvmes::make_unique<Disassembler>(std::move(program2));

    AST ast;
    try {
        ast = d->Disassemble();
    }
    catch (ParseException& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    auto c = llvmes::make_unique<Compiler>(std::move(ast), "load_store");
    c->Compile();

    bool optimized = true;
    c->GetMain(optimized)();

    return 0;
}
