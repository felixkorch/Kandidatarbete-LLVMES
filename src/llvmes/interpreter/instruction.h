#pragma once
#include <string>

namespace llvmes {
    class CPU;
    // Pointer to a function which will execute an instruction
    typedef void(CPU::*OpFunction)();
    // Pointer to a function which will fetch an address used in an instruction
    typedef void(CPU::*AddrFunction)();

    struct Instruction {
        AddrFunction addr;
        OpFunction op;
        std::string name;
    };
}
