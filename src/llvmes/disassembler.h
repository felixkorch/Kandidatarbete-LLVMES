#pragma once

#include <iomanip>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "llvmes/6502_opcode.h"
#include "llvmes/common.h"

namespace llvmes {

enum class StatementType { DataWord, DataByte, Instruction, Label };

enum class InstructionType {
    ImmediateInstruction,
    ImpliedInstruction,
    DirectWithLabelIndexedInstruction,
    DirectIndexedInstruction,
    DirectWithLabelInstruction,
    DirectInstruction,
    IndirectXInstruction,
    IndirectYInstruction,
    IndirectInstruction
};

struct Statement {
    virtual StatementType GetType() = 0;
    virtual void Print() = 0;
    virtual int GetIndex() = 0;
    virtual ~Statement() = default;
};

struct DataWord : public Statement {
    uint16_t offset = 0;
    uint16_t word = 0;

    constexpr DataWord(uint8_t a1, uint8_t a2) : word((a1 << 8) | a2) {}

    explicit constexpr DataWord(uint16_t word) : word(word) {}

    StatementType GetType() override { return StatementType::DataWord; }

    void Print() override
    {
        std::cout << "[" << offset << "]: "
                  << ".dw " << ToHexString(word) << std::endl;
    }

    int GetIndex() override { return offset; }

    friend std::ostream& operator<<(std::ostream& os, const DataWord& dw);
};

std::ostream& operator<<(std::ostream& os, const DataWord& dw)
{
    os << ToHexString(dw.word);
    return os;
}

struct DataByte : public Statement {
    uint16_t offset = 0;
    uint8_t byte = 0;

    explicit constexpr DataByte(uint8_t a) : byte(a) {}

    StatementType GetType() override { return StatementType::DataByte; }

    void Print() override
    {
        std::cout << "[" << offset << "]: "
                  << ".db " << ToHexString(byte) << std::endl;
    }

    int GetIndex() override { return offset; }

    friend std::ostream& operator<<(std::ostream& os, const DataByte& db);
};

std::ostream& operator<<(std::ostream& os, const DataByte& db)
{
    os << ToHexString(db.byte);
    return os;
}

struct Instruction : public Statement {
    int size = 0;
    int offset = 0;
    std::string name;
    MOS6502::AddressingMode addressing_mode = {};
    MOS6502::Op op_type = {};
    uint8_t opcode = 0xFF;
    uint8_t arg[2] = {0};

    StatementType GetType() override { return StatementType::Instruction; }

    void Print() override
    {
        std::cout << "[" << offset << "]: " << ToHexString(opcode) << std::endl;
    }

    int GetIndex() override { return offset; }
};

struct Label : public Statement {
    std::string name;
    uint16_t offset = 0;

    Label(const std::string& name) : name(name) {}

    StatementType GetType() override { return StatementType::Label; }

    void Print() override { std::cout << name << ":" << std::endl; }

    int GetIndex() override { return offset; }
};

class Disassembler {
    using AST_Iterator = std::list<Scope<Statement>>::iterator;
    using AST = std::list<Scope<Statement>>;

    int index = 0x0000; // Right now program must start at address 0
    std::vector<uint8_t> data;
    AST ast;
    AST_Iterator ast_it;
    std::queue<AST_Iterator> branches;

   public:
    Disassembler(std::vector<uint8_t>&& data) : data(data) {}

    AST_Iterator FindStmtByIndex(uint16_t index)
    {
        auto stmt = std::find_if(ast.begin(), ast.end(),
                                 [index](const Scope<Statement>& stmt) {
                                     return stmt->GetIndex() == index;
                                 });

        if (stmt == ast.end())
            std::cout << "Statement at offset " << index << " doesn't exist"
                      << std::endl;

        return stmt;
    }

    AST_Iterator InsertLabelBefore(uint16_t index, const std::string& name)
    {
        auto stmt = FindStmtByIndex(index);
        Scope<Label> lbl = CreateScope<Label>(name);
        lbl->offset = index;
        ast.insert(stmt, std::move(lbl));
        return stmt;
    }

    void ReplaceWithInstruction(AST_Iterator it)
    {
        while (it != ast.end()) {
            // Already an instruction, mark the branch as complete
            if ((*it)->GetType() == StatementType::Instruction)
                break;
            // Don't replace labels
            else if ((*it)->GetType() == StatementType::Label)
                it = std::next(it);

            // Get offset of current statement
            uint16_t offs = (*it)->GetIndex();
            // Get the opcode from the ROM
            uint8_t opcode = data[offs];

            // This contains information about the instruction
            auto instr_info = MOS6502::DecodeInstruction(opcode);

            Scope<Instruction> new_instr = CreateScope<Instruction>();
            new_instr->offset = offs;
            new_instr->name = "Instr";
            new_instr->size = AddressingModeSize(instr_info.addr_mode);
            new_instr->addressing_mode = instr_info.addr_mode;
            new_instr->op_type = instr_info.op;
            new_instr->opcode = opcode;

            // Get a pointer since it will be moved into the AST
            Instruction* instr = new_instr.get();
            *it = std::move(new_instr);

            // Delete the "argument databytes" and replace with instruction
            // e.g. if length is 3 then the 2 bytes that carries data will get
            // removed from the AST since they belong to the instruction
            for (int i = 1; i < instr->size; i++) {
                auto arg = std::next(it);
                DataByte& db = (DataByte&)*arg->get();
                instr->arg[i - 1] = db.byte;
                ast.erase(arg);
            }

            if (IsBranch(instr_info.op) || instr->opcode == 0x4C) {
                uint16_t target_index = 0;

                if (instr_info.op == MOS6502::Op::JSR ||
                    instr->opcode == 0x4C) {  // JSR / JMP Abs
                    target_index = (uint16_t)*instr->arg;
                }
                else {  // Normal branch
                    target_index = (int8_t)*instr->arg + offs + 2;
                }

                std::stringstream ss;
                ss << "Label " << ToHexString(target_index);
                AST_Iterator branch_target =
                    InsertLabelBefore(target_index, ss.str());
                branches.push(branch_target);

                if (instr->opcode == 0x4C)
                    break;
            }

            it = std::next(it);
        }
        branches.pop();
    }

    void Disassemble()
    {
        // Set all the statements to "data bytes"
        for (int i = 0; i < data.size(); i++) {
            Scope<DataByte> db = CreateScope<DataByte>(data[i]);
            db->offset = i;
            ast.insert(ast.end(), std::move(db));
        }

        ast_it = InsertLabelBefore(index, "Reset_Routine");

        ReplaceWithInstruction(ast_it);
        while (!branches.empty())
            ReplaceWithInstruction(branches.front());
    }

    void PrintAST()
    {
        for (const Scope<Statement>& stmt : ast)
            stmt->Print();
    }
};
}  // namespace llvmes