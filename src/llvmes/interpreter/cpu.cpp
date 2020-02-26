#include "llvmes/interpreter/cpu.h"

#include <bitset>
#include <iostream>
#include <map>

namespace llvmes {
CPU::CPU()
    : reg_x(0),
      reg_y(0),
      reg_a(0),
      reg_sp(0xFD),
      reg_pc(0),
      reg_status(0x34),
      m_instruction_table(0x100),
      m_irq(false),
      m_nmi(false),
      m_illegal_opcode(false),
      m_address(0)
{
    for (auto& it : m_instruction_table)
        it = {&CPU::AddressModeImplied, &CPU::IllegalOP, "Illegal OP"};

    m_instruction_table[0xD0] = {&CPU::AddressModeImmediate, &CPU::OP_BNE,
                                 "BNE"};
    m_instruction_table[0xF0] = {&CPU::AddressModeImmediate, &CPU::OP_BEQ,
                                 "BEQ"};
    m_instruction_table[0x30] = {&CPU::AddressModeImmediate, &CPU::OP_BMI,
                                 "BMI"};
    m_instruction_table[0x90] = {&CPU::AddressModeImmediate, &CPU::OP_BCC,
                                 "BCC"};
    m_instruction_table[0xB0] = {&CPU::AddressModeImmediate, &CPU::OP_BCS,
                                 "BCS"};
    m_instruction_table[0x10] = {&CPU::AddressModeImmediate, &CPU::OP_BPL,
                                 "BPL"};
    m_instruction_table[0x50] = {&CPU::AddressModeImmediate, &CPU::OP_BVC,
                                 "BVC"};
    m_instruction_table[0x70] = {&CPU::AddressModeImmediate, &CPU::OP_BVS,
                                 "BVS"};

    m_instruction_table[0xE8] = {&CPU::AddressModeImplied, &CPU::OP_INX, "INX"};
    m_instruction_table[0xC8] = {&CPU::AddressModeImplied, &CPU::OP_INY, "INY"};
    m_instruction_table[0x88] = {&CPU::AddressModeImplied, &CPU::OP_DEY, "DEY"};
    m_instruction_table[0xCA] = {&CPU::AddressModeImplied, &CPU::OP_DEX, "DEX"};

    m_instruction_table[0xE6] = {&CPU::AddressModeZeropage, &CPU::OP_INC,
                                 "INC"};
    m_instruction_table[0xF6] = {&CPU::AddressModeZeropageX, &CPU::OP_INC,
                                 "INC"};
    m_instruction_table[0xEE] = {&CPU::AddressModeAbsolute, &CPU::OP_INC,
                                 "INC"};
    m_instruction_table[0xFE] = {&CPU::AddressModeAbsoluteX, &CPU::OP_INC,
                                 "INC"};

    m_instruction_table[0x4C] = {&CPU::AddressModeAbsolute, &CPU::OP_JMP,
                                 "JMP"};
    m_instruction_table[0x6C] = {&CPU::AddressModeIndirect, &CPU::OP_JMP,
                                 "JMP"};
    m_instruction_table[0x20] = {&CPU::AddressModeAbsolute, &CPU::OP_JSR,
                                 "JSR"};

    m_instruction_table[0x24] = {&CPU::AddressModeZeropage, &CPU::OP_BIT,
                                 "BIT"};
    m_instruction_table[0x2C] = {&CPU::AddressModeAbsolute, &CPU::OP_BIT,
                                 "BIT"};

    m_instruction_table[0x00] = {&CPU::AddressModeImplied, &CPU::OP_BRK, "BRK"};

    m_instruction_table[0x69] = {&CPU::AddressModeImmediate, &CPU::OP_ADC,
                                 "ADC"};

    m_instruction_table[0xC9] = {&CPU::AddressModeImmediate, &CPU::OP_CMP,
                                 "CMP"};
    m_instruction_table[0xC5] = {&CPU::AddressModeZeropage, &CPU::OP_CMP,
                                 "CMP"};
    m_instruction_table[0xD5] = {&CPU::AddressModeZeropageX, &CPU::OP_CMP,
                                 "CMP"};
    m_instruction_table[0xCD] = {&CPU::AddressModeAbsolute, &CPU::OP_CMP,
                                 "CMP"};
    m_instruction_table[0xDD] = {&CPU::AddressModeAbsoluteX, &CPU::OP_CMP,
                                 "CMP"};
    m_instruction_table[0xD9] = {&CPU::AddressModeAbsoluteY, &CPU::OP_CMP,
                                 "CMP"};
    m_instruction_table[0xC1] = {&CPU::AddressModeIndirectX, &CPU::OP_CMP,
                                 "CMP"};
    m_instruction_table[0xD1] = {&CPU::AddressModeIndirectY, &CPU::OP_CMP,
                                 "CMP"};

    m_instruction_table[0xE0] = {&CPU::AddressModeImmediate, &CPU::OP_CPX,
                                 "CPX"};
    m_instruction_table[0xE4] = {&CPU::AddressModeZeropage, &CPU::OP_CPX,
                                 "CPX"};
    m_instruction_table[0xEC] = {&CPU::AddressModeAbsolute, &CPU::OP_CPX,
                                 "CPX"};

    m_instruction_table[0xC0] = {&CPU::AddressModeImmediate, &CPU::OP_CPY,
                                 "CPY"};
    m_instruction_table[0xC4] = {&CPU::AddressModeZeropage, &CPU::OP_CPY,
                                 "CPY"};
    m_instruction_table[0xCC] = {&CPU::AddressModeAbsolute, &CPU::OP_CPY,
                                 "CPY"};

    m_instruction_table[0xC6] = {&CPU::AddressModeZeropage, &CPU::OP_DEC,
                                 "DEC"};
    m_instruction_table[0xD6] = {&CPU::AddressModeZeropageX, &CPU::OP_DEC,
                                 "DEC"};
    m_instruction_table[0xCE] = {&CPU::AddressModeAbsolute, &CPU::OP_DEC,
                                 "DEC"};
    m_instruction_table[0xDE] = {&CPU::AddressModeAbsoluteX, &CPU::OP_DEC,
                                 "DEC"};

    m_instruction_table[0x49] = {&CPU::AddressModeImmediate, &CPU::OP_EOR,
                                 "EOR"};
    m_instruction_table[0x45] = {&CPU::AddressModeZeropage, &CPU::OP_EOR,
                                 "EOR"};
    m_instruction_table[0x55] = {&CPU::AddressModeZeropageX, &CPU::OP_EOR,
                                 "EOR"};
    m_instruction_table[0x4D] = {&CPU::AddressModeAbsolute, &CPU::OP_EOR,
                                 "EOR"};
    m_instruction_table[0x5D] = {&CPU::AddressModeAbsoluteX, &CPU::OP_EOR,
                                 "EOR"};
    m_instruction_table[0x59] = {&CPU::AddressModeAbsoluteY, &CPU::OP_EOR,
                                 "EOR"};
    m_instruction_table[0x41] = {&CPU::AddressModeIndirectX, &CPU::OP_EOR,
                                 "EOR"};
    m_instruction_table[0x51] = {&CPU::AddressModeIndirectY, &CPU::OP_EOR,
                                 "EOR"};

    m_instruction_table[0xA9] = {&CPU::AddressModeImmediate, &CPU::OP_LDA,
                                 "LDA"};
    m_instruction_table[0xA5] = {&CPU::AddressModeZeropage, &CPU::OP_LDA,
                                 "LDA"};
    m_instruction_table[0xB5] = {&CPU::AddressModeZeropageX, &CPU::OP_LDA,
                                 "LDA"};
    m_instruction_table[0xA1] = {&CPU::AddressModeIndirectX, &CPU::OP_LDA,
                                 "LDA"};
    m_instruction_table[0xB1] = {&CPU::AddressModeIndirectY, &CPU::OP_LDA,
                                 "LDA"};
    m_instruction_table[0xAD] = {&CPU::AddressModeAbsolute, &CPU::OP_LDA,
                                 "LDA"};
    m_instruction_table[0xBD] = {&CPU::AddressModeAbsoluteX, &CPU::OP_LDA,
                                 "LDA"};
    m_instruction_table[0xB9] = {&CPU::AddressModeAbsoluteY, &CPU::OP_LDA,
                                 "LDA"};

    m_instruction_table[0xA2] = {&CPU::AddressModeImmediate, &CPU::OP_LDX,
                                 "LDX"};
    m_instruction_table[0xA6] = {&CPU::AddressModeZeropage, &CPU::OP_LDX,
                                 "LDX"};
    m_instruction_table[0xB6] = {&CPU::AddressModeZeropageY, &CPU::OP_LDX,
                                 "LDX"};
    m_instruction_table[0xAE] = {&CPU::AddressModeAbsolute, &CPU::OP_LDX,
                                 "LDX"};
    m_instruction_table[0xBE] = {&CPU::AddressModeAbsoluteY, &CPU::OP_LDX,
                                 "LDX"};

    m_instruction_table[0xA0] = {&CPU::AddressModeImmediate, &CPU::OP_LDY,
                                 "LDY"};
    m_instruction_table[0xA4] = {&CPU::AddressModeZeropage, &CPU::OP_LDY,
                                 "LDY"};
    m_instruction_table[0xB4] = {&CPU::AddressModeZeropageX, &CPU::OP_LDY,
                                 "LDY"};
    m_instruction_table[0xAC] = {&CPU::AddressModeAbsolute, &CPU::OP_LDY,
                                 "LDY"};
    m_instruction_table[0xBC] = {&CPU::AddressModeAbsoluteX, &CPU::OP_LDY,
                                 "LDY"};

    m_instruction_table[0x4A] = {&CPU::AddressModeAccumulator, &CPU::OP_LSR_ACC,
                                 "LSR"};
    m_instruction_table[0x46] = {&CPU::AddressModeZeropage, &CPU::OP_LSR,
                                 "LSR"};
    m_instruction_table[0x56] = {&CPU::AddressModeZeropageX, &CPU::OP_LSR,
                                 "LSR"};
    m_instruction_table[0x4E] = {&CPU::AddressModeAbsolute, &CPU::OP_LSR,
                                 "LSR"};
    m_instruction_table[0x5E] = {&CPU::AddressModeAbsoluteX, &CPU::OP_LSR,
                                 "LSR"};

    m_instruction_table[0x09] = {&CPU::AddressModeImmediate, &CPU::OP_ORA,
                                 "ORA"};
    m_instruction_table[0x05] = {&CPU::AddressModeZeropage, &CPU::OP_ORA,
                                 "ORA"};
    m_instruction_table[0x15] = {&CPU::AddressModeZeropageX, &CPU::OP_ORA,
                                 "ORA"};
    m_instruction_table[0x0D] = {&CPU::AddressModeAbsolute, &CPU::OP_ORA,
                                 "ORA"};
    m_instruction_table[0x1D] = {&CPU::AddressModeAbsoluteX, &CPU::OP_ORA,
                                 "ORA"};
    m_instruction_table[0x19] = {&CPU::AddressModeAbsoluteY, &CPU::OP_ORA,
                                 "ORA"};
    m_instruction_table[0x01] = {&CPU::AddressModeIndirectX, &CPU::OP_ORA,
                                 "ORA"};
    m_instruction_table[0x11] = {&CPU::AddressModeIndirectY, &CPU::OP_ORA,
                                 "ORA"};

    m_instruction_table[0x48] = {&CPU::AddressModeImplied, &CPU::OP_PHA, "PHA"};
    m_instruction_table[0x08] = {&CPU::AddressModeImplied, &CPU::OP_PHP, "PHP"};
    m_instruction_table[0x68] = {&CPU::AddressModeImplied, &CPU::OP_PLA, "PLA"};
    m_instruction_table[0x28] = {&CPU::AddressModeImplied, &CPU::OP_PLP, "PLP"};

    m_instruction_table[0x2A] = {&CPU::AddressModeAccumulator, &CPU::OP_ROL_ACC,
                                 "ROL"};
    m_instruction_table[0x26] = {&CPU::AddressModeZeropage, &CPU::OP_ROL,
                                 "ROL"};
    m_instruction_table[0x36] = {&CPU::AddressModeZeropageX, &CPU::OP_ROL,
                                 "ROL"};
    m_instruction_table[0x2E] = {&CPU::AddressModeAbsolute, &CPU::OP_ROL,
                                 "ROL"};
    m_instruction_table[0x3E] = {&CPU::AddressModeAbsoluteX, &CPU::OP_ROL,
                                 "ROL"};

    m_instruction_table[0x6A] = {&CPU::AddressModeAccumulator, &CPU::OP_ROR_ACC,
                                 "ROR"};
    m_instruction_table[0x66] = {&CPU::AddressModeZeropage, &CPU::OP_ROR,
                                 "ROR"};
    m_instruction_table[0x76] = {&CPU::AddressModeZeropageX, &CPU::OP_ROR,
                                 "ROR"};
    m_instruction_table[0x6E] = {&CPU::AddressModeAbsolute, &CPU::OP_ROR,
                                 "ROR"};
    m_instruction_table[0x7E] = {&CPU::AddressModeAbsoluteX, &CPU::OP_ROR,
                                 "ROR"};

    m_instruction_table[0x40] = {&CPU::AddressModeImplied, &CPU::OP_RTI, "RTI"};
    m_instruction_table[0x60] = {&CPU::AddressModeImplied, &CPU::OP_RTS, "RTS"};

    m_instruction_table[0xE9] = {&CPU::AddressModeImmediate, &CPU::OP_SBC,
                                 "SBC"};
    m_instruction_table[0xE5] = {&CPU::AddressModeZeropage, &CPU::OP_SBC,
                                 "SBC"};
    m_instruction_table[0xF5] = {&CPU::AddressModeZeropageX, &CPU::OP_SBC,
                                 "SBC"};
    m_instruction_table[0xED] = {&CPU::AddressModeAbsolute, &CPU::OP_SBC,
                                 "SBC"};
    m_instruction_table[0xFD] = {&CPU::AddressModeAbsoluteX, &CPU::OP_SBC,
                                 "SBC"};
    m_instruction_table[0xF9] = {&CPU::AddressModeAbsoluteY, &CPU::OP_SBC,
                                 "SBC"};
    m_instruction_table[0xE1] = {&CPU::AddressModeIndirectX, &CPU::OP_SBC,
                                 "SBC"};
    m_instruction_table[0xF1] = {&CPU::AddressModeIndirectY, &CPU::OP_SBC,
                                 "SBC"};

    m_instruction_table[0x38] = {&CPU::AddressModeImplied, &CPU::OP_SEC, "SEC"};
    m_instruction_table[0xF8] = {&CPU::AddressModeImplied, &CPU::OP_SED, "SED"};
    m_instruction_table[0x78] = {&CPU::AddressModeImplied, &CPU::OP_SEI, "SEI"};

    m_instruction_table[0x18] = {&CPU::AddressModeImplied, &CPU::OP_CLC, "CLC"};
    m_instruction_table[0xD8] = {&CPU::AddressModeImplied, &CPU::OP_CLD, "CLD"};
    m_instruction_table[0x58] = {&CPU::AddressModeImplied, &CPU::OP_CLI, "CLI"};
    m_instruction_table[0xB8] = {&CPU::AddressModeImplied, &CPU::OP_CLV, "CLV"};

    m_instruction_table[0x85] = {&CPU::AddressModeZeropage, &CPU::OP_STA,
                                 "STA"};
    m_instruction_table[0x95] = {&CPU::AddressModeZeropageX, &CPU::OP_STA,
                                 "STA"};
    m_instruction_table[0x8D] = {&CPU::AddressModeAbsolute, &CPU::OP_STA,
                                 "STA"};
    m_instruction_table[0x9D] = {&CPU::AddressModeAbsoluteX, &CPU::OP_STA,
                                 "STA"};
    m_instruction_table[0x99] = {&CPU::AddressModeAbsoluteY, &CPU::OP_STA,
                                 "STA"};
    m_instruction_table[0x81] = {&CPU::AddressModeIndirectX, &CPU::OP_STA,
                                 "STA"};
    m_instruction_table[0x91] = {&CPU::AddressModeIndirectY, &CPU::OP_STA,
                                 "STA"};

    m_instruction_table[0x86] = {&CPU::AddressModeZeropage, &CPU::OP_STX,
                                 "STX"};
    m_instruction_table[0x96] = {&CPU::AddressModeZeropageY, &CPU::OP_STX,
                                 "STX"};
    m_instruction_table[0x8E] = {&CPU::AddressModeAbsolute, &CPU::OP_STX,
                                 "STX"};

    m_instruction_table[0x84] = {&CPU::AddressModeZeropage, &CPU::OP_STY,
                                 "STY"};
    m_instruction_table[0x94] = {&CPU::AddressModeZeropageX, &CPU::OP_STY,
                                 "STY"};
    m_instruction_table[0x8C] = {&CPU::AddressModeAbsolute, &CPU::OP_STY,
                                 "STY"};

    m_instruction_table[0xAA] = {&CPU::AddressModeImplied, &CPU::OP_TAX, "TAX"};
    m_instruction_table[0xA8] = {&CPU::AddressModeImplied, &CPU::OP_TAY, "TAY"};
    m_instruction_table[0xBA] = {&CPU::AddressModeImplied, &CPU::OP_TSX, "TSX"};
    m_instruction_table[0x8A] = {&CPU::AddressModeImplied, &CPU::OP_TXA, "TXA"};
    m_instruction_table[0x9A] = {&CPU::AddressModeImplied, &CPU::OP_TXS, "TXS"};
    m_instruction_table[0x98] = {&CPU::AddressModeImplied, &CPU::OP_TYA, "TYA"};

    m_instruction_table[0x29] = {&CPU::AddressModeImmediate, &CPU::OP_AND,
                                 "AND"};
    m_instruction_table[0x25] = {&CPU::AddressModeZeropage, &CPU::OP_AND,
                                 "AND"};
    m_instruction_table[0x35] = {&CPU::AddressModeZeropageX, &CPU::OP_AND,
                                 "AND"};
    m_instruction_table[0x2D] = {&CPU::AddressModeAbsolute, &CPU::OP_AND,
                                 "AND"};
    m_instruction_table[0x3D] = {&CPU::AddressModeAbsoluteX, &CPU::OP_AND,
                                 "AND"};
    m_instruction_table[0x39] = {&CPU::AddressModeAbsoluteY, &CPU::OP_AND,
                                 "AND"};
    m_instruction_table[0x21] = {&CPU::AddressModeIndirectX, &CPU::OP_AND,
                                 "AND"};
    m_instruction_table[0x31] = {&CPU::AddressModeIndirectY, &CPU::OP_AND,
                                 "AND"};

    m_instruction_table[0x0A] = {&CPU::AddressModeAccumulator, &CPU::OP_ASL_ACC,
                                 "ASL"};
    m_instruction_table[0x06] = {&CPU::AddressModeZeropage, &CPU::OP_ASL,
                                 "ASL"};
    m_instruction_table[0x16] = {&CPU::AddressModeZeropageX, &CPU::OP_ASL,
                                 "ASL"};
    m_instruction_table[0x0E] = {&CPU::AddressModeAbsolute, &CPU::OP_ASL,
                                 "ASL"};
    m_instruction_table[0x1E] = {&CPU::AddressModeAbsoluteX, &CPU::OP_ASL,
                                 "ASL"};

    m_instruction_table[0x61] = {&CPU::AddressModeIndirectX, &CPU::OP_ADC,
                                 "ADC"};
    m_instruction_table[0x71] = {&CPU::AddressModeIndirectY, &CPU::OP_ADC,
                                 "ADC"};
    m_instruction_table[0x65] = {&CPU::AddressModeZeropage, &CPU::OP_ADC,
                                 "ADC"};
    m_instruction_table[0x75] = {&CPU::AddressModeZeropageX, &CPU::OP_ADC,
                                 "ADC"};
    m_instruction_table[0x6D] = {&CPU::AddressModeAbsolute, &CPU::OP_ADC,
                                 "ADC"};
    m_instruction_table[0x7D] = {&CPU::AddressModeAbsoluteX, &CPU::OP_ADC,
                                 "ADC"};
    m_instruction_table[0x79] = {&CPU::AddressModeAbsoluteY, &CPU::OP_ADC,
                                 "ADC"};

    m_instruction_table[0xEA] = {&CPU::AddressModeImplied, &CPU::OP_NOP, "NOP"};
}

void CPU::InvokeIRQ()
{
    StackPush(reg_pc >> 8);
    StackPush(reg_pc & 0xFF);
    StackPush(reg_status & ~FLAG_B);
    reg_status.I = 1;
    reg_pc = Read16(IRQ_VECTOR);
}

void CPU::InvokeNMI()
{
    StackPush(reg_pc >> 8);
    StackPush(reg_pc & 0xFF);
    StackPush(reg_status & ~FLAG_B);
    reg_status.I = 1;
    reg_pc = Read16(NMI_VECTOR);
    m_nmi = false;
}

std::uint16_t CPU::Read16(std::uint16_t addr)
{
    std::uint16_t lowByte = Read(addr);
    std::uint16_t highByte = Read(addr + 1);
    return lowByte | (highByte << 8);
}

/// The operand immediately following the opcode
void CPU::AddressModeImmediate()
{
    m_address = reg_pc++;
}

/// The address to the operand is the 2 bytes succeeding the opcode
void CPU::AddressModeAbsolute()
{
    m_address = Read16(reg_pc);
    reg_pc += 2;
}

/// The address to the operand is the 2 bytes succeeding the opcode + value of
/// register X
void CPU::AddressModeAbsoluteX()
{
    m_address = Read16(reg_pc) + reg_x;
    reg_pc += 2;
}

/// The address to the operand is the 2 bytes succeeding the opcode + value of
/// register Y
void CPU::AddressModeAbsoluteY()
{
    m_address = Read16(reg_pc) + reg_y;
    reg_pc += 2;
}

/// The address to the operand is the byte succeeding the opcode extended to
/// 16bits
void CPU::AddressModeZeropage()
{
    m_address = Read(reg_pc++);
}

///-Indexed
/// The address to the operand is the byte succeeding the opcode + register X
/// If the address gets larger than 0x100(255) the address will wrap and gets
/// back to 0
void CPU::AddressModeZeropageX()
{
    std::uint8_t addr = ((Read(reg_pc++) + reg_x) % 0x100);
    m_address = addr;
}

/// Y-Indexed
/// The address to the operand is the byte succeeding the opcode + register Y
/// If the address gets larger than 0x100(255) the address will wrap and gets
/// back to 0
void CPU::AddressModeZeropageY()
{
    std::uint8_t addr = ((Read(reg_pc++) + reg_y) % 0x100);
    m_address = addr;
}

/// The two bytes that follow the opcode is an address which contains the
/// LS-Byte of the real target address. The other byte is located in "target
/// address + 1". Due to an error in the original design, if the target address
/// is located on a page-boundary, the last byte of the address will be on
/// 0xYY00
void CPU::AddressModeIndirect()
{
    std::uint16_t indirection = Read16(reg_pc);
    std::uint8_t low = Read(indirection);
    std::uint8_t high =
        Read((0xFF00 & indirection) | ((indirection + 1) % 0x100));
    m_address = low | (high << 8);
}

void CPU::AddressModeIndirectX()
{
    // This address is used to index the
    std::uint8_t base = (Read(reg_pc++) + reg_x);
    std::uint8_t low = Read(base);
    // Wrap if the address gets to 255
    std::uint8_t high = Read(base + 1);
    m_address = low | (high << 8);
}

void CPU::AddressModeIndirectY()
{
    // This address is used to index the
    std::uint8_t base = (Read(reg_pc++));
    std::uint8_t low = Read(base);
    // Wrap if the address gets to 255
    std::uint8_t high = Read(base + 1);
    // Add the contents of Y to get the final address
    m_address = (low | (high << 8)) + reg_y;
}

void CPU::AddressModeImplied()
{
    // Simply means the instruction doesn't need an operand
}

void CPU::AddressModeAccumulator()
{
    // The operand is the contents of the accumulator(regA)
}

void CPU::StackPush(std::uint8_t value)
{
    Write(0x0100 | reg_sp--, value);
}

std::uint8_t CPU::StackPop()
{
    return Read(0x0100 | ++reg_sp);
}

void CPU::SetNMI()
{
    m_nmi = true;
}

void CPU::SetIRQ()
{
    m_irq = true;
}

void CPU::Step()
{
    // Interrupt handling
    if (m_nmi)
        InvokeNMI();
    else if (m_irq && !reg_status.I)
        InvokeIRQ();

    // Fetch
    std::uint8_t opcode = Read(reg_pc++);

    // Decode
    Instruction& instr = m_instruction_table[opcode];

    // Execute
    (this->*instr.fetch_address)();  // Fetch the address (if necessary)
    (this->*instr.op)();             // Execute the instruction
}

void CPU::Dump()
{
    std::cout << "Register X: " << (unsigned int)reg_x << "\n"
              << "Register Y: " << (unsigned int)reg_y << "\n"
              << "Register A: " << (unsigned int)reg_a << "\n"
              << "Register SP: " << (unsigned int)reg_sp << "\n"
              << "Register PC: " << std::hex << reg_pc << "\n"
              << "Flags: " << std::bitset<8>(reg_status) << std::dec << "\n\n";
}

void CPU::Reset()
{
    reg_pc = Read16(RESET_VECTOR);
    reg_status = 0x34;
    reg_x = 0;
    reg_y = 0;
    reg_a = 0;
    reg_sp = 0xFD;
    m_address = 0;
    m_nmi = false;
    m_irq = false;
    m_illegal_opcode = false;
}

void CPU::Run()
{
    while (!m_illegal_opcode) Step();
}

void CPU::IllegalOP()
{
    m_illegal_opcode = true;
}

// A + M + C -> A, C
void CPU::OP_ADC()
{
    std::uint8_t operand = Read(m_address);
    std::uint32_t result = reg_a + operand + reg_status.C;
    bool overflow = !((reg_a ^ operand) & 0x80) && ((reg_a ^ result) & 0x80);
    reg_status.Z = (result & 0xFF) == 0;
    reg_status.C = result > 0xFF;
    reg_status.V = overflow;
    reg_status.N = result & 0x80;
    reg_a = result & 0xFF;
}

void CPU::OP_BRK()
{
    StackPush((reg_pc + 1) >> 8);
    StackPush((reg_pc + 1) & 0xFF);
    StackPush(reg_status | FLAG_B | FLAG_UNUSED);
    reg_status.I = 1;
    reg_pc = Read16(IRQ_VECTOR);
}

void CPU::OP_INC()
{
    std::uint8_t operand = Read(m_address);
    operand++;
    Write(m_address, operand);
    reg_status.Z = operand == 0;
    reg_status.N = operand & 0x80;
}

void CPU::OP_DEC()
{
    std::uint8_t operand = Read(m_address);
    operand--;
    Write(m_address, operand);
    reg_status.Z = operand == 0;
    reg_status.N = operand & 0x80;
}

void CPU::OP_INX()
{
    reg_x++;
    reg_status.Z = reg_x == 0;
    reg_status.N = reg_x & 0x80;
}

void CPU::OP_INY()
{
    reg_y++;
    reg_status.Z = reg_y == 0;
    reg_status.N = reg_y & 0x80;
}

void CPU::OP_DEY()
{
    reg_y--;
    reg_status.Z = reg_y == 0;
    reg_status.N = reg_y & 0x80;
}

void CPU::OP_DEX()
{
    reg_x--;
    reg_status.Z = reg_x == 0;
    reg_status.N = reg_x & 0x80;
}

void CPU::OP_NOP()
{
    // No operation
}

void CPU::OP_LDY()
{
    // Load index Y with memory
    std::uint8_t operand = Read(m_address);
    reg_y = operand;
    reg_status.Z = operand == 0;
    reg_status.N = operand & 0x80;
}

void CPU::OP_LDA()
{
    // Load Accumulator
    std::uint8_t operand = Read(m_address);
    reg_a = operand;
    reg_status.Z = operand == 0;
    reg_status.N = operand & 0x80;
}

void CPU::OP_LDX()
{
    // Load Accumulator
    std::uint8_t operand = Read(m_address);
    reg_x = operand;
    reg_status.Z = operand == 0;
    reg_status.N = operand & 0x80;
}

void CPU::OP_JMP()
{
    reg_pc = m_address;
}

void CPU::OP_JSR()
{
    std::uint16_t returnAddress =
        reg_pc - 1;  // TODO: Should be just regPC since we increment in step()?
    StackPush(returnAddress >> 8);  // Push PC high
    StackPush(returnAddress);       // Push PC low
    reg_pc = m_address;
}

void CPU::OP_BNE()
{
    std::int8_t operand = Read(m_address);
    if (!reg_status.Z)
        reg_pc += operand;
}

void CPU::OP_BEQ()
{
    std::int8_t operand = Read(m_address);
    if (reg_status.Z)
        reg_pc += operand;
}

void CPU::OP_BMI()
{
    std::int8_t operand = Read(m_address);
    if (reg_status.N)
        reg_pc += operand;
}

void CPU::OP_BCC()
{
    std::int8_t operand = Read(m_address);
    if (!reg_status.C)
        reg_pc += operand;
}

void CPU::OP_BCS()
{
    std::int8_t operand = Read(m_address);
    if (reg_status.C)
        reg_pc += operand;
}

void CPU::OP_BPL()
{
    std::int8_t operand = Read(m_address);
    if (!reg_status.N)
        reg_pc += operand;
}

void CPU::OP_BVC()
{
    std::int8_t operand = Read(m_address);
    if (!reg_status.V)
        reg_pc += operand;
}

void CPU::OP_BVS()
{
    std::int8_t operand = Read(m_address);
    if (reg_status.V)
        reg_pc += operand;
}

void CPU::OP_SEI()
{
    reg_status = reg_status | FLAG_I;
}
void CPU::OP_CLI()
{
    reg_status = reg_status & ~FLAG_I;
}

void CPU::OP_CLC()
{
    reg_status = reg_status & ~FLAG_C;
}

void CPU::OP_CLD()
{
    reg_status = reg_status & ~FLAG_D;
}

void CPU::OP_CLV()
{
    reg_status = reg_status & ~FLAG_V;
}

void CPU::OP_BIT()
{
    std::uint8_t operand = Read(m_address);
    reg_status.N = operand & 0x80;
    reg_status.V = operand & 0x40;
    reg_status.Z = (operand & reg_a) == 0;
}

void CPU::OP_EOR()
{
    std::uint8_t operand = Read(m_address);
    reg_a ^= operand;
    reg_status.Z = reg_a == 0;
    reg_status.N = reg_a & 0x80;
}

void CPU::OP_AND()
{
    std::uint8_t operand = Read(m_address);
    reg_a &= operand;
    reg_status.Z = reg_a == 0;
    reg_status.N = reg_a & 0x80;
}

void CPU::OP_ASL()
{
    std::uint8_t operand = Read(m_address);
    reg_status.C = operand & 0x80;
    operand <<= 1;
    Write(m_address, operand);
    reg_status.Z = operand == 0;
    reg_status.N = operand & 0x80;
}
void CPU::OP_ASL_ACC()
{
    reg_status.C = reg_a & 0x80;
    reg_a <<= 1;
    reg_status.Z = reg_a == 0;
    reg_status.N = reg_a & 0x80;
}

void CPU::OP_LSR()
{
    std::uint8_t operand = Read(m_address);
    reg_status.C = operand & 1;
    operand >>= 1;
    Write(m_address, operand);
    reg_status.Z = operand == 0;
    reg_status.N = 0;
}

void CPU::OP_LSR_ACC()
{
    reg_status.C = reg_a & 1;
    reg_a >>= 1;
    reg_status.Z = reg_a == 0;
    reg_status.N = 0;
}

void CPU::OP_ORA()
{
    std::uint8_t operand = Read(m_address);
    reg_a |= operand;
    reg_status.Z = reg_a == 0;
    reg_status.N = reg_a & 0x80;
}

void CPU::OP_STY()
{
    Write(m_address, reg_y);
}

void CPU::OP_STA()
{
    Write(m_address, reg_a);
}

void CPU::OP_STX()
{
    Write(m_address, reg_x);
}

void CPU::OP_PHA()
{
    StackPush(reg_a);
}

void CPU::OP_PHP()
{
    StackPush(reg_status | FLAG_B | FLAG_UNUSED);
}

void CPU::OP_PLA()
{
    reg_a = StackPop();
    reg_status.Z = reg_a == 0;
    reg_status.N = reg_a & 0x80;
}

void CPU::OP_PLP()
{
    reg_status = StackPop();
}

void CPU::OP_ROL()
{
    std::uint32_t operand = Read(m_address);
    operand <<= 1;
    operand = reg_status.C ? operand | 1 : operand & ~1;
    reg_status.C = operand & 0x0100;
    reg_status.Z = (operand & 0xFF) == 0;
    reg_status.N = (operand & 0xFF) & 0x80;
    Write(m_address, operand & 0xFF);
}

void CPU::OP_ROL_ACC()
{
    std::uint32_t operand = reg_a;
    operand <<= 1;
    operand = reg_status.C ? operand | 1 : operand & ~1;
    reg_status.C = operand & 0x0100;
    reg_status.Z = (operand & 0xFF) == 0;
    reg_status.N = (operand & 0xFF) & 0x80;
    reg_a = operand & 0xFF;
}
void CPU::OP_ROR_ACC()
{
    std::uint32_t operand = reg_a;
    operand >>= 1;
    operand = reg_status.C ? operand | 0x0100 : operand & ~0x0100;
    reg_status.C = operand & 1;
    reg_status.Z = (operand & 0xFF) == 0;
    reg_status.N = (operand & 0xFF) & 0x80;
    reg_a = operand & 0xFF;
}

void CPU::OP_ROR()
{
    std::uint32_t operand = Read(m_address);
    operand = reg_status.C ? operand | 0x0100 : operand & ~0x0100;
    operand >>= 1;
    reg_status.C = operand & 1;
    reg_status.Z = (operand & 0xFF) == 0;
    reg_status.N = (operand & 0xFF) & 0x80;
    Write(m_address, operand & 0xFF);
}

void CPU::OP_RTI()
{
    reg_status = StackPop();
    reg_pc = StackPop() | (StackPop() << 8);
}

void CPU::OP_RTS()
{
    reg_pc = (StackPop() | (StackPop() << 8)) + 1;
}

// A - M - C -> A
void CPU::OP_SBC()
{
    std::uint8_t operand = Read(m_address);
    std::uint32_t result = reg_a - operand - !reg_status.C;
    bool overflow = ((reg_a ^ result) & 0x80) && ((reg_a ^ operand) & 0x80);
    reg_status.Z = (result & 0xFF) == 0;
    reg_status.C = result < 0x0100;
    reg_status.V = overflow;
    reg_status.N = result & 0x80;
    reg_a = result & 0xFF;
}

void CPU::OP_SEC()
{
    reg_status.C = 1;
}

void CPU::OP_SED()
{
    reg_status.D = 1;
}

void CPU::OP_TAX()
{
    reg_x = reg_a;
    reg_status.Z = reg_x == 0;
    reg_status.N = reg_x & 0x80;
}

void CPU::OP_TSX()
{
    reg_x = reg_sp;
    reg_status.Z = reg_x == 0;
    reg_status.N = reg_x & 0x80;
}

void CPU::OP_TYA()
{
    reg_a = reg_y;
    reg_status.Z = reg_a == 0;
    reg_status.N = reg_a & 0x80;
}

void CPU::OP_TXS()
{
    reg_sp = reg_x;
}

void CPU::OP_TXA()
{
    reg_a = reg_x;
    reg_status.Z = reg_a == 0;
    reg_status.N = reg_a & 0x80;
}

void CPU::OP_TAY()
{
    reg_y = reg_a;
    reg_status.Z = reg_y == 0;
    reg_status.N = reg_y & 0x80;
}

// A - M
void CPU::OP_CMP()
{
    std::uint8_t operand = Read(m_address);
    std::uint32_t result = reg_a - operand;
    reg_status.Z = reg_a == operand;
    reg_status.N = result & 0x80;
    reg_status.C = result < 0x0100;
}
// X - M
void CPU::OP_CPX()
{
    std::uint8_t operand = Read(m_address);
    std::uint32_t result = reg_x - operand;
    reg_status.Z = reg_x == operand;
    reg_status.N = result & 0x80;
    reg_status.C = result < 0x0100;
}

// Y - M
void CPU::OP_CPY()
{
    std::uint8_t operand = Read(m_address);
    std::uint32_t result = reg_y - operand;
    reg_status.Z = reg_y == operand;
    reg_status.N = result & 0x80;
    reg_status.C = result < 0x0100;
}

DisassemblyMap CPU::Disassemble(std::uint16_t start, std::uint16_t stop)
{
    // Contains the final map
    std::map<std::uint16_t, std::string> map;

    // Local PC to step through the memory while disassembling
    std::uint32_t pc = start;

    // Contains the operand associated with an instruction
    std::uint32_t operand;

    while (pc <= (std::uint32_t)stop) {
        // The address to the instruction (opcode)
        std::uint16_t instr_addr = pc;

        // The dissasembly string for the instruction
        std::string instr_string = "";

        std::uint8_t opcode = Read(pc++);

        instr_string += m_instruction_table[opcode].name + " ";

        if (m_instruction_table[opcode].fetch_address ==
            &CPU::AddressModeImplied) {
            instr_string += " [IMP]";
        }
        else if (m_instruction_table[opcode].fetch_address ==
                 &CPU::AddressModeImmediate) {
            operand = Read(pc++);
            instr_string += "#" + ToHexString<std::uint8_t>(operand) + " [IMM]";
        }
        else if (m_instruction_table[opcode].fetch_address ==
                 &CPU::AddressModeZeropage) {
            operand = Read(pc++);
            instr_string += ToHexString<std::uint8_t>(operand) + " [ZP]";
        }
        else if (m_instruction_table[opcode].fetch_address ==
                 &CPU::AddressModeZeropageX) {
            operand = Read(pc++);
            instr_string += ToHexString<std::uint8_t>(operand) + ", X [ZPX]";
        }
        else if (m_instruction_table[opcode].fetch_address ==
                 &CPU::AddressModeZeropageY) {
            operand = Read(pc++);
            instr_string += ToHexString<std::uint8_t>(operand) + ", Y [ZPY]";
        }
        else if (m_instruction_table[opcode].fetch_address ==
                 &CPU::AddressModeIndirectX) {
            operand = Read(pc++);
            instr_string +=
                "(" + ToHexString<std::uint8_t>(operand) + ", X) [IZX]";
        }
        else if (m_instruction_table[opcode].fetch_address ==
                 &CPU::AddressModeIndirectY) {
            operand = Read(pc++);
            instr_string +=
                "(" + ToHexString<std::uint8_t>(operand) + "), Y [IZY]";
        }
        else if (m_instruction_table[opcode].fetch_address ==
                 &CPU::AddressModeAbsolute) {
            operand = Read16(pc);
            pc += 2;
            instr_string += ToHexString<std::uint16_t>(operand) + " [ABS]";
        }
        else if (m_instruction_table[opcode].fetch_address ==
                 &CPU::AddressModeAbsoluteX) {
            operand = Read16(pc);
            pc += 2;
            instr_string += ToHexString<std::uint16_t>(operand) + ", X [ABX]";
        }
        else if (m_instruction_table[opcode].fetch_address ==
                 &CPU::AddressModeAbsoluteY) {
            operand = Read16(pc);
            pc += 2;
            instr_string += ToHexString<std::uint16_t>(operand) + ", Y [ABY]";
        }
        else if (m_instruction_table[opcode].fetch_address ==
                 &CPU::AddressModeIndirect) {
            operand = Read16(pc);
            pc += 2;
            instr_string +=
                "(" + ToHexString<std::uint16_t>(operand) + ") [IND]";
        }

        map[instr_addr] = instr_string;
    }

    return map;
}

}  // namespace llvmes
