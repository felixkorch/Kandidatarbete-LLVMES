#pragma once

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "llvmes/common.h"
#include "llvmes/dynarec/6502_opcode.h"
#include "time.h"

namespace llvmes {

struct Instruction {
    int size = 0;
    uint16_t offset = 0;
    uint8_t opcode = 0xFF;
    uint16_t arg = 0;
    std::string name;
    MOS6502::AddressingMode addressing_mode = {};
    MOS6502::Op op_type = {};

    std::string target_label;

    void Print()
    {
        std::cout << "[" << ToHexString(offset) << "]: " << ToHexString(opcode)
                  << std::endl;
    }

    int GetOffset() { return offset; }
};

class ParseException : public std::exception {
   public:
    explicit ParseException(const char* message) : msg(message) {}

    explicit ParseException(const std::string& message) : msg(message) {}

    virtual ~ParseException() throw() {}

    virtual const char* what() const throw() { return msg.c_str(); }

   protected:
    std::string msg;
};

struct AST {
    std::unordered_map<uint16_t, std::string> labels;
    std::map<uint16_t, Instruction*> instructions;
};

class Parser {
    std::vector<uint8_t> data;
    size_t program_size;
    uint16_t start_location;
    uint16_t reset_address;
    uint16_t index;

    std::unordered_map<uint16_t, std::string> labels;
    std::map<uint16_t, Instruction*> instructions;
    std::queue<uint16_t> branches;

    void ParseInstructions(uint16_t start);
    uint16_t ParseArgument(Instruction* instruction);
    std::string AddLabel(Instruction* instruction);

   public:
    Parser(std::vector<uint8_t>&& data_in, uint16_t start_location);
    std::vector<uint8_t> GetRAM() { return data; }
    AST Parse();
};
}  // namespace llvmes
