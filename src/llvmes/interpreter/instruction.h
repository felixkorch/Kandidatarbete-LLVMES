#pragma once
#include <string>

namespace llvmes {
class CPU;
// Pointer to a function which will execute an instruction
typedef void (CPU::*Operation_FnPtr)();
// Pointer to a function which will fetch an address used in an instruction
typedef void (CPU::*AddressMode_FnPtr)();

struct Instruction {
    Operation_FnPtr fetch_address;
    AddressMode_FnPtr op;
    std::string name;
};
}  // namespace llvmes
