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
namespace dynarec {

struct Label {
    uint16_t address;
    std::string name;
    operator uint16_t() { return address; }
};

struct Instruction {
    int size = 0;
    uint16_t offset = 0;
    uint8_t opcode = 0xFF;
    uint16_t arg = 0;
    std::string name;
    MOS6502::AddressingMode addressing_mode = {};
    MOS6502::Op op_type = {};

    Label target_label;
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

struct ParseResult {
    std::map<uint16_t, Label> labels;
    std::map<uint16_t, Instruction*> instructions;
    std::vector<uint8_t> memory;
};

class Parser {
    std::vector<uint8_t> data;
    size_t program_size;
    uint16_t start_location;
    uint16_t reset_address;
    uint16_t index;

    std::map<uint16_t, Label> labels;
    std::map<uint16_t, Instruction*> instructions;
    std::queue<uint16_t> branches;

    void ParseInstructions(uint16_t start);
    uint16_t ParseArgument(Instruction* instruction);
    Label AddLabel(Instruction* instruction);

   public:
    Parser(std::vector<uint8_t>&& data_in, uint16_t start_location);
    ParseResult Parse();
};
}  // namespace dynarec
}  // namespace llvmes
