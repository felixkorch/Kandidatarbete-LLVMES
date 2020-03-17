#pragma once
#include <vector>
#include <unordered_map>
#include <map>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <queue>

#include "6502_opcode.h"

template<typename T>
using Scope = std::unique_ptr<T>;
template<typename T, typename ... Args>
constexpr Scope<T> CreateScope(Args&& ... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename T>
inline std::string ToHexString(T i)
{
    std::stringstream stream;
    stream << "$" << std::uppercase << std::setfill('0')
           << std::setw(sizeof(T) * 2) << std::hex << (unsigned)i;
    return stream.str();
}

template <>
inline std::string ToHexString<bool>(bool i)
{
    std::stringstream stream;
    stream << std::uppercase << std::setw(1) << std::hex << i;
    return stream.str();
}

inline int HexStringToInt(const std::string& in)
{
    auto strip_zeroes = [](std::string temp) {
        while (temp.at(0) == '0') temp = temp.substr(1);
        return std::move(temp);
    };

    std::string out;
    out = (in.at(0) == '$') ? strip_zeroes(in.substr(1)) : strip_zeroes(in);
    return std::stoi(out, 0, 16);
}

enum class StatementType {
    DataWord, DataByte, Instruction, Label
};

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

using namespace llvmes;

struct Statement {
    virtual StatementType GetType() = 0;
    virtual void Print() = 0;
    virtual int GetIndex() = 0;
    virtual ~Statement() = default;
};

struct DataWord : public Statement {
    uint16_t index = 0;
    uint16_t word = 0;

    DataWord(std::initializer_list<uint8_t> l)
    : word((*l.begin() << 8) | (*l.begin() + 1)) {}

    DataWord(uint8_t a1, uint8_t a2)
    : word((a1 << 8) | a2)
    {}

    StatementType GetType() override { return StatementType::DataWord; }
    void Print() override
    {
        std::cout << ".dw " << ToHexString(word) << std::endl;
    }

    int GetIndex() override
    {
        return index;
    }
};

struct DataByte : public Statement {
    uint16_t index = 0;
    uint8_t byte = 0;

    DataByte(uint8_t a)
    : byte(a) {}

    DataByte(std::initializer_list<uint8_t> l)
        : byte(*l.begin()) {}

    StatementType GetType() override { return StatementType::DataByte; }

    void Print() override
    {
        std::cout << "[" << index << "]: " << ".db " << ToHexString(byte) << std::endl;
    }

    int GetIndex() override
    {
        return index;
    }
};

struct Instruction : public Statement {
    int size;
    int index;
    std::string name;
    AdressingMode addressing_mode;
    Op op_type;
    uint8_t opcode;
    uint8_t arg[2];

    StatementType GetType() override { return StatementType::Instruction; }

    void Print() override
    {
        std::cout << "[" << index << "]: " << ToHexString(opcode) << std::endl;
    }

    int GetIndex() override
    {
        return index;
    }
};

struct Label : public Statement {
    std::string name;
    uint16_t index = 0;

    Label(const std::string& name)
    : name(name) {}

    StatementType GetType() override { return StatementType::Label; }

    void Print() override
    {
        std::cout << name << ":" << std::endl;
    }

    int GetIndex() override
    {
        return index;
    }
};

class Disassembler {
    static constexpr uint16_t start_index = 0x0000;
    using AST_Iterator = std::list<Scope<Statement>>::iterator;
    using AST = std::list<Scope<Statement>>;

    int index = start_index;
    std::vector<uint8_t> data;
    AST ast;
    AST_Iterator ast_it;
    std::queue<AST_Iterator> branches;
public:
    Disassembler(std::vector<uint8_t>&& data)
        : data(data)
    {}

    AST_Iterator FindStmtByIndex(uint16_t index)
    {
        auto stmt = std::find_if(ast.begin(), ast.end(),
                [index](const Scope<Statement>& stmt){ return stmt->GetIndex() == index; });

        if(stmt == ast.end())
            std::cout << "Statement at index " << index << " doesn't exist" << std::endl;

        return stmt;
    }

    AST_Iterator InsertLabelBefore(uint16_t index, const std::string& name)
    {
        auto stmt = FindStmtByIndex(index);
        Scope<Label> lbl = CreateScope<Label>(name);
        lbl->index = index;
        ast.insert(stmt, std::move(lbl));
        return stmt;
    }


    void FillBranchWithInstructions(AST_Iterator it)
    {

        while(it != ast.end()) {

            if((*it)->GetType() == StatementType::Instruction) {
                branches.pop();
                return;
            }

            uint16_t offs = (*it)->GetIndex(); // Get index of current statement
            uint8_t opcode = data[offs]; // Get the opcode from the ROM

            // This contains information about the instruction
            auto instr_info = decode_MOS6502_instruction(opcode);

            Scope<Instruction> new_instr = CreateScope<Instruction>();
            new_instr->index = offs;
            new_instr->name = "Instr";
            new_instr->size = MOS6502_addressing_mode_size(instr_info.addr_mode);
            new_instr->addressing_mode = instr_info.addr_mode;
            new_instr->op_type = instr_info.op;
            new_instr->opcode = opcode;

            // Get a pointer since it will be moved into the AST
            Instruction* instr = new_instr.get();
            *it = std::move(new_instr);

            // Delete the "argument databytes" and replace with instruction
            // e.g. if length is 3 then the 2 bytes that carries data will get removed from the AST
            // since they belong to the instruction
            for(int i = 1; i < instr->size; i++) {
                auto arg = std::next(it);
                DataByte& db = (DataByte&)*arg->get();
                instr->arg[i - 1] = db.byte;
                ast.erase(arg);
            }

            if(MOS6502_opcode_is_branch(instr_info.op) || instr->op_type == Op::JMP) {
                uint16_t target_index = 0;

                if(instr_info.op == Op::JSR || instr->opcode == 0x4C) { // JSR / JMP Abs
                    target_index = (uint16_t)*instr->arg;
                }
                else { // Normal branch
                    target_index = (int8_t)instr->arg[0] + offs + 1;
                }

                std::stringstream ss;
                ss << "Label " << ToHexString(target_index);
                AST_Iterator branch_target = InsertLabelBefore(target_index, ss.str());
                branches.push(branch_target);

                if(instr->opcode == 0x4C)
                    break;
            }

            it = std::next(it);
        }
        branches.pop();
    }

    void Disassemble()
    {
        // Set all the statements to "data bytes"
        for(int i = 0; i < data.size(); i++) {
            Scope<DataByte> db = CreateScope<DataByte>(data[i]);
            db->index = i;
            ast.insert(ast.end(), std::move(db));
        }

        ast_it = InsertLabelBefore(index, "Reset_Routine");

        FillBranchWithInstructions(ast_it);
        while(!branches.empty())
            FillBranchWithInstructions(branches.front());
    }

    void PrintAST()
    {
        for(const Scope<Statement>& stmt : ast)
            stmt->Print();
    }


};