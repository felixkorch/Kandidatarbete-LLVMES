#include "llvmes/dynarec/parser.h"

namespace llvmes {

Parser::Parser(std::vector<uint8_t>&& data_in, uint16_t start_location)
    : data(0x10000),
      program_size(data_in.size()),
      start_location(start_location)
{
    auto temp = std::move(data_in);
    if (start_location + program_size >= 0xFFFF)
        throw ParseException("Program doesn't fit in that space");
    std::copy(temp.begin(), temp.end(), data.begin() + start_location);
}

// TODO: Maybe break this up a bit to increase readability
void Parser::ParseInstructions(uint16_t start)
{
    index = start;
    while (1) {
        // Already parsed
        if (instructions.count(index))
            break;

        uint8_t opcode = data[index];

        // This contains information about the instruction
        MOS6502::Instruction mos_instr = MOS6502::DecodeInstruction(opcode);
        Instruction* instr = new Instruction;

        instr->offset = index;
        instr->name = "Instr";  // TODO: Add instruction names
        instr->size = MOS6502::InstructionSize(mos_instr.addr_mode);
        instr->addressing_mode = mos_instr.addr_mode;
        instr->op_type = mos_instr.op;
        instr->opcode = opcode;

        assert(instr->size == 1 || instr->size == 2 || instr->size == 3);

        if (instr->size == 2) {
            if (index + 1 >= data.size())
                throw ParseException("Machine code has illegal format");

            instr->arg = data[index + 1];
        }
        else if (instr->size == 3) {
            if (index + 2 >= data.size())
                throw ParseException("Machine code has illegal format");

            instr->arg = data[index + 1] | data[index + 2] << 8;
        }

        instructions[index] = instr;

        // According to our ABI, this is a return statement
        if (instr->opcode == 0x8D && instr->arg == 0x200F)
            break;

        bool JMP_Abs = instr->opcode == 0x4C;
        if (IsBranch(mos_instr.op) || JMP_Abs) {
            instr->is_branchinstruction = true;
            uint16_t target_index = 0;

            // Unconditional branch
            if (mos_instr.op == MOS6502::Op::JSR || JMP_Abs) {
                target_index = instr->arg;
            }
            // Conditional branch
            else {
                target_index = (int8_t)instr->arg + index + 2;
            }

            // Only add label if it doesn't exist
            if (labels.count(target_index) == 0) {
                std::stringstream ss;
                ss << "Label " << ToHexString(target_index);
                branches.push(target_index);
                instr->target_label = ss.str();
                labels[target_index] = ss.str();
            }
            else {
                instr->target_label = labels[target_index];
            }

            // JMP ends a branch
            if (JMP_Abs)
                break;
        }

        index += instr->size;
    }
}

AST Parser::Parse()
{
    uint16_t reset = data[0xFFFC] | (data[0xFFFD] << 8);
    reset_address = reset;
    labels[reset_address] = "Reset";
    index = reset_address;
    branches.push(index);

    do {
        ParseInstructions(branches.front());
        branches.pop();
    } while (!branches.empty());

    return {labels, instructions};
}

}  // namespace llvmes