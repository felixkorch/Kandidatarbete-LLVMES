#pragma once

namespace llvmes {
namespace MOS6502 {

enum class Op {
    BIT,
    AND,
    EOR,
    ORA,
    ASL,
    ASL_ACC,
    LSR,
    LSR_ACC,
    ADC,
    JSR,
    JMP,
    BNE,
    BEQ,
    BMI,
    BCC,
    BCS,
    BPL,
    BVC,
    BVS,
    BRK,
    LDY,
    LDA,
    LDX,
    INX,
    INC,
    DEC,
    INY,
    DEY,
    DEX,
    NOP,
    SEI,
    CLI,
    CLC,
    CLD,
    CLV,
    PHA,
    PHP,
    PLA,
    PLP,
    ROL,
    ROR,
    ROL_ACC,
    ROR_ACC,
    RTI,
    RTS,
    SBC,
    SEC,
    SED,
    STA,
    STX,
    STY,
    TAX,
    TAY,
    TSX,
    TYA,
    TXS,
    TXA,
    CMP,
    CPX,
    CPY,
    IllegalOP,
};

enum class AddressingMode {
    Immediate,
    Absolute,
    AbsoluteX,
    AbsoluteY,
    Zeropage,
    ZeropageX,
    ZeropageY,
    Indirect,
    IndirectX,
    IndirectY,
    Implied,
    Accumulator,
};

struct Instruction {
    Op op;
    AddressingMode addr_mode;
};

// Technically undefined to run a branch instruction inside a basic block.
// At the very least, it must not branch, so ... it's effectively a no-op.
bool IsBranch(Op op)
{
    switch (op) {
        case Op::JSR:
        case Op::BNE:
        case Op::BEQ:
        case Op::BMI:
        case Op::BCC:
        case Op::BCS:
        case Op::BPL:
        case Op::BVC:
        case Op::BVS:
            return true;
        default:
            return false;
    }
}

int AddressingModeSize(AddressingMode mode)
{
    switch (mode) {
        case AddressingMode::Immediate:
            return 2;
        case AddressingMode::Absolute:
            return 3;
        case AddressingMode::AbsoluteX:
            return 3;
        case AddressingMode::AbsoluteY:
            return 3;
        case AddressingMode::Zeropage:
            return 2;
        case AddressingMode::ZeropageX:
            return 2;
        case AddressingMode::ZeropageY:
            return 2;
        case AddressingMode::Indirect:
            return 3;
        case AddressingMode::IndirectX:
            return 2;
        case AddressingMode::IndirectY:
            return 2;
        case AddressingMode::Implied:
            return 1;
        case AddressingMode::Accumulator:
            return 1;
        default:
            return 1;
    }
}

inline Instruction DecodeInstruction(uint8_t opcode)
{
    Instruction instr = {};
    instr.op = Op::IllegalOP;

    switch (opcode) {
        case 0xD0:
            instr.addr_mode = AddressingMode::Immediate;
            instr.op = Op::BNE;
            break;
        case 0xF0:
            instr.addr_mode = AddressingMode::Immediate;
            instr.op = Op::BEQ;
            break;
        case 0x30:
            instr.addr_mode = AddressingMode::Immediate;
            instr.op = Op::BMI;
            break;
        case 0x90:
            instr.addr_mode = AddressingMode::Immediate;
            instr.op = Op::BCC;
            break;
        case 0xB0:
            instr.addr_mode = AddressingMode::Immediate;
            instr.op = Op::BCS;
            break;
        case 0x10:
            instr.addr_mode = AddressingMode::Immediate;
            instr.op = Op::BPL;
            break;
        case 0x50:
            instr.addr_mode = AddressingMode::Immediate;
            instr.op = Op::BVC;
            break;
        case 0x70:
            instr.addr_mode = AddressingMode::Immediate;
            instr.op = Op::BVS;
            break;
        case 0xE8:
            instr.addr_mode = AddressingMode::Implied;
            instr.op = Op::INX;
            break;
        case 0xC8:
            instr.addr_mode = AddressingMode::Implied;
            instr.op = Op::INY;
            break;
        case 0x88:
            instr.addr_mode = AddressingMode::Implied;
            instr.op = Op::DEY;
            break;
        case 0xCA:
            instr.addr_mode = AddressingMode::Implied;
            instr.op = Op::DEX;
            break;
        case 0xE6:
            instr.addr_mode = AddressingMode::Zeropage;
            instr.op = Op::INC;
            break;
        case 0xF6:
            instr.addr_mode = AddressingMode::ZeropageX;
            instr.op = Op::INC;
            break;
        case 0xEE:
            instr.addr_mode = AddressingMode::Absolute;
            instr.op = Op::INC;
            break;
        case 0xFE:
            instr.addr_mode = AddressingMode::AbsoluteX;
            instr.op = Op::INC;
            break;
        case 0x4C:
            instr.addr_mode = AddressingMode::Absolute;
            instr.op = Op::JMP;
            break;
        case 0x6C:
            instr.addr_mode = AddressingMode::Indirect;
            instr.op = Op::JMP;
            break;
        case 0x20:
            instr.addr_mode = AddressingMode::Absolute;
            instr.op = Op::JSR;
            break;
        case 0x24:
            instr.addr_mode = AddressingMode::Zeropage;
            instr.op = Op::BIT;
            break;
        case 0x2C:
            instr.addr_mode = AddressingMode::Absolute;
            instr.op = Op::BIT;
            break;
        case 0x00:
            instr.addr_mode = AddressingMode::Implied;
            instr.op = Op::BRK;
            break;
        case 0x69:
            instr.addr_mode = AddressingMode::Immediate;
            instr.op = Op::ADC;
            break;
        case 0xC9:
            instr.addr_mode = AddressingMode::Immediate;
            instr.op = Op::CMP;
            break;
        case 0xC5:
            instr.addr_mode = AddressingMode::Zeropage;
            instr.op = Op::CMP;
            break;
        case 0xD5:
            instr.addr_mode = AddressingMode::ZeropageX;
            instr.op = Op::CMP;
            break;
        case 0xCD:
            instr.addr_mode = AddressingMode::Absolute;
            instr.op = Op::CMP;
            break;
        case 0xDD:
            instr.addr_mode = AddressingMode::AbsoluteX;
            instr.op = Op::CMP;
            break;
        case 0xD9:
            instr.addr_mode = AddressingMode::AbsoluteY;
            instr.op = Op::CMP;
            break;
        case 0xC1:
            instr.addr_mode = AddressingMode::IndirectX;
            instr.op = Op::CMP;
            break;
        case 0xD1:
            instr.addr_mode = AddressingMode::IndirectY;
            instr.op = Op::CMP;
            break;
        case 0xE0:
            instr.addr_mode = AddressingMode::Immediate;
            instr.op = Op::CPX;
            break;
        case 0xE4:
            instr.addr_mode = AddressingMode::Zeropage;
            instr.op = Op::CPX;
            break;
        case 0xEC:
            instr.addr_mode = AddressingMode::Absolute;
            instr.op = Op::CPX;
            break;
        case 0xC0:
            instr.addr_mode = AddressingMode::Immediate;
            instr.op = Op::CPY;
            break;
        case 0xC4:
            instr.addr_mode = AddressingMode::Zeropage;
            instr.op = Op::CPY;
            break;
        case 0xCC:
            instr.addr_mode = AddressingMode::Absolute;
            instr.op = Op::CPY;
            break;
        case 0xC6:
            instr.addr_mode = AddressingMode::Zeropage;
            instr.op = Op::DEC;
            break;
        case 0xD6:
            instr.addr_mode = AddressingMode::ZeropageX;
            instr.op = Op::DEC;
            break;
        case 0xCE:
            instr.addr_mode = AddressingMode::Absolute;
            instr.op = Op::DEC;
            break;
        case 0xDE:
            instr.addr_mode = AddressingMode::AbsoluteX;
            instr.op = Op::DEC;
            break;
        case 0x49:
            instr.addr_mode = AddressingMode::Immediate;
            instr.op = Op::EOR;
            break;
        case 0x45:
            instr.addr_mode = AddressingMode::Zeropage;
            instr.op = Op::EOR;
            break;
        case 0x55:
            instr.addr_mode = AddressingMode::ZeropageX;
            instr.op = Op::EOR;
            break;
        case 0x4D:
            instr.addr_mode = AddressingMode::Absolute;
            instr.op = Op::EOR;
            break;
        case 0x5D:
            instr.addr_mode = AddressingMode::AbsoluteX;
            instr.op = Op::EOR;
            break;
        case 0x59:
            instr.addr_mode = AddressingMode::AbsoluteY;
            instr.op = Op::EOR;
            break;
        case 0x41:
            instr.addr_mode = AddressingMode::IndirectX;
            instr.op = Op::EOR;
            break;
        case 0x51:
            instr.addr_mode = AddressingMode::IndirectY;
            instr.op = Op::EOR;
            break;
        case 0xA9:
            instr.addr_mode = AddressingMode::Immediate;
            instr.op = Op::LDA;
            break;
        case 0xA5:
            instr.addr_mode = AddressingMode::Zeropage;
            instr.op = Op::LDA;
            break;
        case 0xB5:
            instr.addr_mode = AddressingMode::ZeropageX;
            instr.op = Op::LDA;
            break;
        case 0xA1:
            instr.addr_mode = AddressingMode::IndirectX;
            instr.op = Op::LDA;
            break;
        case 0xB1:
            instr.addr_mode = AddressingMode::IndirectY;
            instr.op = Op::LDA;
            break;
        case 0xAD:
            instr.addr_mode = AddressingMode::Absolute;
            instr.op = Op::LDA;
            break;
        case 0xBD:
            instr.addr_mode = AddressingMode::AbsoluteX;
            instr.op = Op::LDA;
            break;
        case 0xB9:
            instr.addr_mode = AddressingMode::AbsoluteY;
            instr.op = Op::LDA;
            break;
        case 0xA2:
            instr.addr_mode = AddressingMode::Immediate;
            instr.op = Op::LDX;
            break;
        case 0xA6:
            instr.addr_mode = AddressingMode::Zeropage;
            instr.op = Op::LDX;
            break;
        case 0xB6:
            instr.addr_mode = AddressingMode::ZeropageY;
            instr.op = Op::LDX;
            break;
        case 0xAE:
            instr.addr_mode = AddressingMode::Absolute;
            instr.op = Op::LDX;
            break;
        case 0xBE:
            instr.addr_mode = AddressingMode::AbsoluteY;
            instr.op = Op::LDX;
            break;
        case 0xA0:
            instr.addr_mode = AddressingMode::Immediate;
            instr.op = Op::LDY;
            break;
        case 0xA4:
            instr.addr_mode = AddressingMode::Zeropage;
            instr.op = Op::LDY;
            break;
        case 0xB4:
            instr.addr_mode = AddressingMode::ZeropageX;
            instr.op = Op::LDY;
            break;
        case 0xAC:
            instr.addr_mode = AddressingMode::Absolute;
            instr.op = Op::LDY;
            break;
        case 0xBC:
            instr.addr_mode = AddressingMode::AbsoluteX;
            instr.op = Op::LDY;
            break;
        case 0x4A:
            instr.addr_mode = AddressingMode::Accumulator;
            instr.op = Op::LSR_ACC;
            break;
        case 0x46:
            instr.addr_mode = AddressingMode::Zeropage;
            instr.op = Op::LSR;
            break;
        case 0x56:
            instr.addr_mode = AddressingMode::ZeropageX;
            instr.op = Op::LSR;
            break;
        case 0x4E:
            instr.addr_mode = AddressingMode::Absolute;
            instr.op = Op::LSR;
            break;
        case 0x5E:
            instr.addr_mode = AddressingMode::AbsoluteX;
            instr.op = Op::LSR;
            break;
        case 0x09:
            instr.addr_mode = AddressingMode::Immediate;
            instr.op = Op::ORA;
            break;
        case 0x05:
            instr.addr_mode = AddressingMode::Zeropage;
            instr.op = Op::ORA;
            break;
        case 0x15:
            instr.addr_mode = AddressingMode::ZeropageX;
            instr.op = Op::ORA;
            break;
        case 0x0D:
            instr.addr_mode = AddressingMode::Absolute;
            instr.op = Op::ORA;
            break;
        case 0x1D:
            instr.addr_mode = AddressingMode::AbsoluteX;
            instr.op = Op::ORA;
            break;
        case 0x19:
            instr.addr_mode = AddressingMode::AbsoluteY;
            instr.op = Op::ORA;
            break;
        case 0x01:
            instr.addr_mode = AddressingMode::IndirectX;
            instr.op = Op::ORA;
            break;
        case 0x11:
            instr.addr_mode = AddressingMode::IndirectY;
            instr.op = Op::ORA;
            break;
        case 0x48:
            instr.addr_mode = AddressingMode::Implied;
            instr.op = Op::PHA;
            break;
        case 0x08:
            instr.addr_mode = AddressingMode::Implied;
            instr.op = Op::PHP;
            break;
        case 0x68:
            instr.addr_mode = AddressingMode::Implied;
            instr.op = Op::PLA;
            break;
        case 0x28:
            instr.addr_mode = AddressingMode::Implied;
            instr.op = Op::PLP;
            break;
        case 0x2A:
            instr.addr_mode = AddressingMode::Accumulator;
            instr.op = Op::ROL_ACC;
            break;
        case 0x26:
            instr.addr_mode = AddressingMode::Zeropage;
            instr.op = Op::ROL;
            break;
        case 0x36:
            instr.addr_mode = AddressingMode::ZeropageX;
            instr.op = Op::ROL;
            break;
        case 0x2E:
            instr.addr_mode = AddressingMode::Absolute;
            instr.op = Op::ROL;
            break;
        case 0x3E:
            instr.addr_mode = AddressingMode::AbsoluteX;
            instr.op = Op::ROL;
            break;
        case 0x6A:
            instr.addr_mode = AddressingMode::Accumulator;
            instr.op = Op::ROR_ACC;
            break;
        case 0x66:
            instr.addr_mode = AddressingMode::Zeropage;
            instr.op = Op::ROR;
            break;
        case 0x76:
            instr.addr_mode = AddressingMode::ZeropageX;
            instr.op = Op::ROR;
            break;
        case 0x6E:
            instr.addr_mode = AddressingMode::Absolute;
            instr.op = Op::ROR;
            break;
        case 0x7E:
            instr.addr_mode = AddressingMode::AbsoluteX;
            instr.op = Op::ROR;
            break;
        case 0x40:
            instr.addr_mode = AddressingMode::Implied;
            instr.op = Op::RTI;
            break;
        case 0x60:
            instr.addr_mode = AddressingMode::Implied;
            instr.op = Op::RTS;
            break;
        case 0xE9:
            instr.addr_mode = AddressingMode::Immediate;
            instr.op = Op::SBC;
            break;
        case 0xE5:
            instr.addr_mode = AddressingMode::Zeropage;
            instr.op = Op::SBC;
            break;
        case 0xF5:
            instr.addr_mode = AddressingMode::ZeropageX;
            instr.op = Op::SBC;
            break;
        case 0xED:
            instr.addr_mode = AddressingMode::Absolute;
            instr.op = Op::SBC;
            break;
        case 0xFD:
            instr.addr_mode = AddressingMode::AbsoluteX;
            instr.op = Op::SBC;
            break;
        case 0xF9:
            instr.addr_mode = AddressingMode::AbsoluteY;
            instr.op = Op::SBC;
            break;
        case 0xE1:
            instr.addr_mode = AddressingMode::IndirectX;
            instr.op = Op::SBC;
            break;
        case 0xF1:
            instr.addr_mode = AddressingMode::IndirectY;
            instr.op = Op::SBC;
            break;
        case 0x38:
            instr.addr_mode = AddressingMode::Implied;
            instr.op = Op::SEC;
            break;
        case 0xF8:
            instr.addr_mode = AddressingMode::Implied;
            instr.op = Op::SED;
            break;
        case 0x78:
            instr.addr_mode = AddressingMode::Implied;
            instr.op = Op::SEI;
            break;
        case 0x18:
            instr.addr_mode = AddressingMode::Implied;
            instr.op = Op::CLC;
            break;
        case 0xD8:
            instr.addr_mode = AddressingMode::Implied;
            instr.op = Op::CLD;
            break;
        case 0x58:
            instr.addr_mode = AddressingMode::Implied;
            instr.op = Op::CLI;
            break;
        case 0xB8:
            instr.addr_mode = AddressingMode::Implied;
            instr.op = Op::CLV;
            break;
        case 0x85:
            instr.addr_mode = AddressingMode::Zeropage;
            instr.op = Op::STA;
            break;
        case 0x95:
            instr.addr_mode = AddressingMode::ZeropageX;
            instr.op = Op::STA;
            break;
        case 0x8D:
            instr.addr_mode = AddressingMode::Absolute;
            instr.op = Op::STA;
            break;
        case 0x9D:
            instr.addr_mode = AddressingMode::AbsoluteX;
            instr.op = Op::STA;
            break;
        case 0x99:
            instr.addr_mode = AddressingMode::AbsoluteY;
            instr.op = Op::STA;
            break;
        case 0x81:
            instr.addr_mode = AddressingMode::IndirectX;
            instr.op = Op::STA;
            break;
        case 0x91:
            instr.addr_mode = AddressingMode::IndirectY;
            instr.op = Op::STA;
            break;
        case 0x86:
            instr.addr_mode = AddressingMode::Zeropage;
            instr.op = Op::STX;
            break;
        case 0x96:
            instr.addr_mode = AddressingMode::ZeropageY;
            instr.op = Op::STX;
            break;
        case 0x8E:
            instr.addr_mode = AddressingMode::Absolute;
            instr.op = Op::STX;
            break;
        case 0x84:
            instr.addr_mode = AddressingMode::Zeropage;
            instr.op = Op::STY;
            break;
        case 0x94:
            instr.addr_mode = AddressingMode::ZeropageX;
            instr.op = Op::STY;
            break;
        case 0x8C:
            instr.addr_mode = AddressingMode::Absolute;
            instr.op = Op::STY;
            break;
        case 0xAA:
            instr.addr_mode = AddressingMode::Implied;
            instr.op = Op::TAX;
            break;
        case 0xA8:
            instr.addr_mode = AddressingMode::Implied;
            instr.op = Op::TAY;
            break;
        case 0xBA:
            instr.addr_mode = AddressingMode::Implied;
            instr.op = Op::TSX;
            break;
        case 0x8A:
            instr.addr_mode = AddressingMode::Implied;
            instr.op = Op::TXA;
            break;
        case 0x9A:
            instr.addr_mode = AddressingMode::Implied;
            instr.op = Op::TXS;
            break;
        case 0x98:
            instr.addr_mode = AddressingMode::Implied;
            instr.op = Op::TYA;
            break;
        case 0x29:
            instr.addr_mode = AddressingMode::Immediate;
            instr.op = Op::AND;
            break;
        case 0x25:
            instr.addr_mode = AddressingMode::Zeropage;
            instr.op = Op::AND;
            break;
        case 0x35:
            instr.addr_mode = AddressingMode::ZeropageX;
            instr.op = Op::AND;
            break;
        case 0x2D:
            instr.addr_mode = AddressingMode::Absolute;
            instr.op = Op::AND;
            break;
        case 0x3D:
            instr.addr_mode = AddressingMode::AbsoluteX;
            instr.op = Op::AND;
            break;
        case 0x39:
            instr.addr_mode = AddressingMode::AbsoluteY;
            instr.op = Op::AND;
            break;
        case 0x21:
            instr.addr_mode = AddressingMode::IndirectX;
            instr.op = Op::AND;
            break;
        case 0x31:
            instr.addr_mode = AddressingMode::IndirectY;
            instr.op = Op::AND;
            break;
        case 0x0A:
            instr.addr_mode = AddressingMode::Accumulator;
            instr.op = Op::ASL_ACC;
            break;
        case 0x06:
            instr.addr_mode = AddressingMode::Zeropage;
            instr.op = Op::ASL;
            break;
        case 0x16:
            instr.addr_mode = AddressingMode::ZeropageX;
            instr.op = Op::ASL;
            break;
        case 0x0E:
            instr.addr_mode = AddressingMode::Absolute;
            instr.op = Op::ASL;
            break;
        case 0x1E:
            instr.addr_mode = AddressingMode::AbsoluteX;
            instr.op = Op::ASL;
            break;
        case 0x61:
            instr.addr_mode = AddressingMode::IndirectX;
            instr.op = Op::ADC;
            break;
        case 0x71:
            instr.addr_mode = AddressingMode::IndirectY;
            instr.op = Op::ADC;
            break;
        case 0x65:
            instr.addr_mode = AddressingMode::Zeropage;
            instr.op = Op::ADC;
            break;
        case 0x75:
            instr.addr_mode = AddressingMode::ZeropageX;
            instr.op = Op::ADC;
            break;
        case 0x6D:
            instr.addr_mode = AddressingMode::Absolute;
            instr.op = Op::ADC;
            break;
        case 0x7D:
            instr.addr_mode = AddressingMode::AbsoluteX;
            instr.op = Op::ADC;
            break;
        case 0x79:
            instr.addr_mode = AddressingMode::AbsoluteY;
            instr.op = Op::ADC;
            break;
        case 0xEA:
            instr.addr_mode = AddressingMode::Implied;
            instr.op = Op::NOP;
            break;
    }

    return instr;
}

}  // namespace MOS6502
}  // namespace llvmes