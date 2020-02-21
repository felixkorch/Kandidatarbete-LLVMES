#include "llvmes/interpreter/cpu.h"
#include <iostream>
#include <bitset>

namespace llvmes {

    CPU::CPU()
        : regX(0)
        , regY(0)
        , regA(0)
        , regSP(0xFD)
        , regPC(0)
        , regStatus(0x34)
        , instructionTable(0xFF)
        , m_irq(false)
        , m_nmi(false)
        , m_illegal_opcode(false)
        , m_address(0)
    {
        for(auto& it : instructionTable)
            it = {&CPU::AddressModeImplied, &CPU::IllegalOP, "Illegal OP" };

        instructionTable[0xD0] = {&CPU::AddressModeImmediate, &CPU::OP_BNE, "BNE" };
        instructionTable[0xF0] = {&CPU::AddressModeImmediate, &CPU::OP_BEQ, "BEQ" };
        instructionTable[0x30] = {&CPU::AddressModeImmediate, &CPU::OP_BMI, "BMI" };
        instructionTable[0x90] = {&CPU::AddressModeImmediate, &CPU::OP_BCC, "BCC" };
        instructionTable[0xB0] = {&CPU::AddressModeImmediate, &CPU::OP_BCS, "BCS" };
        instructionTable[0x10] = {&CPU::AddressModeImmediate, &CPU::OP_BPL, "BPL" };
        instructionTable[0x50] = {&CPU::AddressModeImmediate, &CPU::OP_BVC, "BVC" };
        instructionTable[0x70] = {&CPU::AddressModeImmediate, &CPU::OP_BVS, "BVS" };

        instructionTable[0xE8] = {&CPU::AddressModeImplied, &CPU::OP_INX, "INX" };
        instructionTable[0xC8] = {&CPU::AddressModeImplied, &CPU::OP_INY, "INY" };
        instructionTable[0x88] = {&CPU::AddressModeImplied, &CPU::OP_DEY, "DEY" };
        instructionTable[0xCA] = {&CPU::AddressModeImplied, &CPU::OP_DEX, "DEX" };

        instructionTable[0xE6] = {&CPU::AddressModeZeropage, &CPU::OP_INC, "INC" };
        instructionTable[0xF6] = {&CPU::AddressModeZeropageX, &CPU::OP_INC, "INC" };
        instructionTable[0xEE] = {&CPU::AddressModeAbsolute, &CPU::OP_INC, "INC" };
        instructionTable[0xFE] = {&CPU::AddressModeAbsoluteX, &CPU::OP_INC, "INC" };

        instructionTable[0x4C] = {&CPU::AddressModeAbsolute, &CPU::OP_JMP, "JMP Abs" };
        instructionTable[0x6C] = {&CPU::AddressModeIndirect, &CPU::OP_JMP, "JMP Indirect" };
        instructionTable[0x20] = {&CPU::AddressModeAbsolute, &CPU::OP_JSR, "JSR" };

        instructionTable[0x24] = {&CPU::AddressModeZeropage, &CPU::OP_BIT, "BIT Zeropage" };
        instructionTable[0x2C] = {&CPU::AddressModeAbsolute, &CPU::OP_BIT, "BIT Abs" };

        instructionTable[0x00] = {&CPU::AddressModeImplied, &CPU::OP_BRK, "BRK" };

        instructionTable[0x69] = {&CPU::AddressModeImmediate, &CPU::OP_ADC, "ADC Imm" };

        instructionTable[0xC9] = {&CPU::AddressModeImmediate, &CPU::OP_CMP, "CMP Imm" };
        instructionTable[0xC5] = {&CPU::AddressModeZeropage, &CPU::OP_CMP, "CMP Zeropage" };
        instructionTable[0xD5] = {&CPU::AddressModeZeropageX, &CPU::OP_CMP, "CMP Zeropage X" };
        instructionTable[0xCD] = {&CPU::AddressModeAbsolute, &CPU::OP_CMP, "CMP Abs" };
        instructionTable[0xDD] = {&CPU::AddressModeAbsoluteX, &CPU::OP_CMP, "CMP Abs X" };
        instructionTable[0xD9] = {&CPU::AddressModeAbsoluteY, &CPU::OP_CMP, "CMP Abs Y" };
        instructionTable[0xC1] = {&CPU::AddressModeIndirectX, &CPU::OP_CMP, "CMP Indirect X" };
        instructionTable[0xD1] = {&CPU::AddressModeIndirectY, &CPU::OP_CMP, "CMP Indirect Y" };

        instructionTable[0xE0] = {&CPU::AddressModeImmediate, &CPU::OP_CPX, "CPX" };
        instructionTable[0xE4] = {&CPU::AddressModeZeropage, &CPU::OP_CPX, "CPX" };
        instructionTable[0xEC] = {&CPU::AddressModeAbsolute, &CPU::OP_CPX, "CPX" };

        instructionTable[0xC0] = {&CPU::AddressModeImmediate, &CPU::OP_CPY, "CPY" };
        instructionTable[0xC4] = {&CPU::AddressModeZeropage, &CPU::OP_CPY, "CPY" };
        instructionTable[0xCC] = {&CPU::AddressModeAbsolute, &CPU::OP_CPY, "CPY" };

        instructionTable[0xC6] = {&CPU::AddressModeZeropage, &CPU::OP_DEC, "DEC" };
        instructionTable[0xD6] = {&CPU::AddressModeZeropageX, &CPU::OP_DEC, "DEC" };
        instructionTable[0xCE] = {&CPU::AddressModeAbsolute, &CPU::OP_DEC, "DEC" };
        instructionTable[0xDE] = {&CPU::AddressModeAbsoluteX, &CPU::OP_DEC, "DEC" };

        instructionTable[0x49] = {&CPU::AddressModeImmediate, &CPU::OP_EOR, "EOR Imm" };
        instructionTable[0x45] = {&CPU::AddressModeZeropage, &CPU::OP_EOR, "EOR Zeropage" };
        instructionTable[0x55] = {&CPU::AddressModeZeropageX, &CPU::OP_EOR, "EOR Zeropage X" };
        instructionTable[0x4D] = {&CPU::AddressModeAbsolute, &CPU::OP_EOR, "EOR Abs" };
        instructionTable[0x5D] = {&CPU::AddressModeAbsoluteX, &CPU::OP_EOR, "EOR Abs X" };
        instructionTable[0x59] = {&CPU::AddressModeAbsoluteY, &CPU::OP_EOR, "EOR Abs Y" };
        instructionTable[0x41] = {&CPU::AddressModeIndirectX, &CPU::OP_EOR, "EOR Indirect X" };
        instructionTable[0x51] = {&CPU::AddressModeIndirectY, &CPU::OP_EOR, "EOR Indirect Y" };

        instructionTable[0xA9] = {&CPU::AddressModeImmediate, &CPU::OP_LDA, "LDA Imm" };
        instructionTable[0xA5] = {&CPU::AddressModeZeropage, &CPU::OP_LDA, "LDA Zeropage" };
        instructionTable[0xB5] = {&CPU::AddressModeZeropageX, &CPU::OP_LDA, "LDA Zeropage X" };
        instructionTable[0xA1] = {&CPU::AddressModeIndirectX, &CPU::OP_LDA, "LDA Indirect X" };
        instructionTable[0xB1] = {&CPU::AddressModeIndirectY, &CPU::OP_LDA, "LDA Indirect Y" };
        instructionTable[0xAD] = {&CPU::AddressModeAbsolute, &CPU::OP_LDA, "LDA Abs" };
        instructionTable[0xBD] = {&CPU::AddressModeAbsoluteX, &CPU::OP_LDA, "LDA Abs X" };
        instructionTable[0xB9] = {&CPU::AddressModeAbsoluteY, &CPU::OP_LDA, "LDA Abs Y" };

        instructionTable[0xA2] = {&CPU::AddressModeImmediate, &CPU::OP_LDX, "LDX Imm" };
        instructionTable[0xA6] = {&CPU::AddressModeZeropage, &CPU::OP_LDX, "LDX Zeropage" };
        instructionTable[0xB6] = {&CPU::AddressModeZeropageY, &CPU::OP_LDX, "LDX Zeropage Y" };
        instructionTable[0xAE] = {&CPU::AddressModeAbsolute, &CPU::OP_LDX, "LDX Abs" };
        instructionTable[0xBE] = {&CPU::AddressModeAbsoluteY, &CPU::OP_LDX, "LDX Abs Y" };

        instructionTable[0xA0] = {&CPU::AddressModeImmediate, &CPU::OP_LDY, "LDY Imm" };
        instructionTable[0xA4] = {&CPU::AddressModeZeropage, &CPU::OP_LDY, "LDY Zeropage" };
        instructionTable[0xB4] = {&CPU::AddressModeZeropageX, &CPU::OP_LDY, "LDY Zeropage X" };
        instructionTable[0xAC] = {&CPU::AddressModeAbsolute, &CPU::OP_LDY, "LDY Abs" };
        instructionTable[0xBC] = {&CPU::AddressModeAbsoluteX, &CPU::OP_LDY, "LDY Abs X" };

        instructionTable[0x4A] = {&CPU::AddressModeAccumulator, &CPU::OP_LSR_ACC, "LSR Acc" };
        instructionTable[0x46] = {&CPU::AddressModeZeropage, &CPU::OP_LSR, "LSR Zeropage" };
        instructionTable[0x56] = {&CPU::AddressModeZeropageX, &CPU::OP_LSR, "LSR Zeropage X" };
        instructionTable[0x4E] = {&CPU::AddressModeAbsolute, &CPU::OP_LSR, "LSR Abs" };
        instructionTable[0x5E] = {&CPU::AddressModeAbsoluteX, &CPU::OP_LSR, "LSR Abs X" };

        instructionTable[0x09] = {&CPU::AddressModeImmediate, &CPU::OP_ORA, "ORA Imm" };
        instructionTable[0x05] = {&CPU::AddressModeZeropage, &CPU::OP_ORA, "ORA Zeropage" };
        instructionTable[0x15] = {&CPU::AddressModeZeropageX, &CPU::OP_ORA, "ORA Zeropage X" };
        instructionTable[0x0D] = {&CPU::AddressModeAbsolute, &CPU::OP_ORA, "ORA Abs" };
        instructionTable[0x1D] = {&CPU::AddressModeAbsoluteX, &CPU::OP_ORA, "ORA Abs X" };
        instructionTable[0x19] = {&CPU::AddressModeAbsoluteY, &CPU::OP_ORA, "ORA Abs Y" };
        instructionTable[0x01] = {&CPU::AddressModeIndirectX, &CPU::OP_ORA, "ORA Indirect X" };
        instructionTable[0x11] = {&CPU::AddressModeIndirectY, &CPU::OP_ORA, "ORA Indirect Y" };

        instructionTable[0x48] = {&CPU::AddressModeImplied, &CPU::OP_PHA, "PHA" };
        instructionTable[0x08] = {&CPU::AddressModeImplied, &CPU::OP_PHP, "PHP" };
        instructionTable[0x68] = {&CPU::AddressModeImplied, &CPU::OP_PLA, "PLA" };
        instructionTable[0x28] = {&CPU::AddressModeImplied, &CPU::OP_PLP, "PLP" };

        instructionTable[0x2A] = {&CPU::AddressModeImplied, &CPU::OP_ROL_ACC, "ROL Acc" };
        instructionTable[0x26] = {&CPU::AddressModeZeropage, &CPU::OP_ROL, "ROL Zeropage" };
        instructionTable[0x36] = {&CPU::AddressModeZeropageX, &CPU::OP_ROL, "ROL Zeropage X" };
        instructionTable[0x2E] = {&CPU::AddressModeAbsolute, &CPU::OP_ROL, "ROL Abs" };
        instructionTable[0x3E] = {&CPU::AddressModeAbsoluteX, &CPU::OP_ROL, "ROL Abs X" };

        instructionTable[0x6A] = {&CPU::AddressModeImplied, &CPU::OP_ROR_ACC, "ROR Acc" };
        instructionTable[0x66] = {&CPU::AddressModeZeropage, &CPU::OP_ROR, "ROR Zeropage" };
        instructionTable[0x76] = {&CPU::AddressModeZeropageX, &CPU::OP_ROR, "ROR Zeropage X" };
        instructionTable[0x6E] = {&CPU::AddressModeAbsolute, &CPU::OP_ROR, "ROR Abs" };
        instructionTable[0x7E] = {&CPU::AddressModeAbsoluteX, &CPU::OP_ROR, "ROR Abs X" };

        instructionTable[0x40] = {&CPU::AddressModeImplied, &CPU::OP_RTI, "RTI" };
        instructionTable[0x60] = {&CPU::AddressModeImplied, &CPU::OP_RTS, "RTS" };

        instructionTable[0xE9] = {&CPU::AddressModeImmediate, &CPU::OP_SBC, "SBC Imm" };
        instructionTable[0xE5] = {&CPU::AddressModeZeropage, &CPU::OP_SBC, "SBC Zeropage" };
        instructionTable[0xF5] = {&CPU::AddressModeZeropageX, &CPU::OP_SBC, "SBC ZeropageX" };
        instructionTable[0xED] = {&CPU::AddressModeAbsolute, &CPU::OP_SBC, "SBC Abs" };
        instructionTable[0xFD] = {&CPU::AddressModeAbsoluteX, &CPU::OP_SBC, "SBC Abs X" };
        instructionTable[0xF9] = {&CPU::AddressModeAbsoluteY, &CPU::OP_SBC, "SBC Abs Y" };
        instructionTable[0xE1] = {&CPU::AddressModeIndirectX, &CPU::OP_SBC, "SBC Indirect X" };
        instructionTable[0xF1] = {&CPU::AddressModeIndirectY, &CPU::OP_SBC, "SBC Indirect Y" };

        instructionTable[0x38] = {&CPU::AddressModeImplied, &CPU::OP_SEC, "SEC" };
        instructionTable[0xF8] = {&CPU::AddressModeImplied, &CPU::OP_SED, "SED" };
        instructionTable[0x78] = {&CPU::AddressModeImplied, &CPU::OP_SEI, "SEI" };

        instructionTable[0x18] = {&CPU::AddressModeImplied, &CPU::OP_CLC, "CLC" };
        instructionTable[0xD8] = {&CPU::AddressModeImplied, &CPU::OP_CLD, "CLD" };
        instructionTable[0x58] = {&CPU::AddressModeImplied, &CPU::OP_CLI, "CLI" };
        instructionTable[0xB8] = {&CPU::AddressModeImplied, &CPU::OP_CLV, "CLV" };

        instructionTable[0x85] = {&CPU::AddressModeZeropage, &CPU::OP_STA, "STA Zeropage" };
        instructionTable[0x95] = {&CPU::AddressModeZeropageX, &CPU::OP_STA, "STA Zeropage X" };
        instructionTable[0x8D] = {&CPU::AddressModeAbsolute, &CPU::OP_STA, "STA Abs" };
        instructionTable[0x9D] = {&CPU::AddressModeAbsoluteX, &CPU::OP_STA, "STA Abs X" };
        instructionTable[0x99] = {&CPU::AddressModeAbsoluteY, &CPU::OP_STA, "STA Abs Y" };
        instructionTable[0x81] = {&CPU::AddressModeIndirectX, &CPU::OP_STA, "STA Indirect X" };
        instructionTable[0x91] = {&CPU::AddressModeIndirectY, &CPU::OP_STA, "STA Indirect Y" };

        instructionTable[0x86] = {&CPU::AddressModeZeropage, &CPU::OP_STX, "STX Zeropage" };
        instructionTable[0x96] = {&CPU::AddressModeZeropageY, &CPU::OP_STX, "STX Zeropage Y" };
        instructionTable[0x8E] = {&CPU::AddressModeAbsolute, &CPU::OP_STX, "STX Abs" };

        instructionTable[0x84] = {&CPU::AddressModeZeropage, &CPU::OP_STY, "STY Zeropage" };
        instructionTable[0x94] = {&CPU::AddressModeZeropageX, &CPU::OP_STY, "STY Zeropage X" };
        instructionTable[0x8C] = {&CPU::AddressModeAbsolute, &CPU::OP_STY, "STY Abs" };

        instructionTable[0xAA] = {&CPU::AddressModeImplied, &CPU::OP_TAX, "TAX" };
        instructionTable[0xA8] = {&CPU::AddressModeImplied, &CPU::OP_TAY, "TAY" };
        instructionTable[0xBA] = {&CPU::AddressModeImplied, &CPU::OP_TSX, "TSX" };
        instructionTable[0x8A] = {&CPU::AddressModeImplied, &CPU::OP_TXA, "TXA" };
        instructionTable[0x9A] = {&CPU::AddressModeImplied, &CPU::OP_TXS, "TXS" };
        instructionTable[0x98] = {&CPU::AddressModeImplied, &CPU::OP_TYA, "TYA" };

        instructionTable[0x29] = {&CPU::AddressModeImmediate, &CPU::OP_AND, "AND Imm" };
        instructionTable[0x25] = {&CPU::AddressModeZeropage, &CPU::OP_AND, "AND Zeropage" };
        instructionTable[0x35] = {&CPU::AddressModeZeropageX, &CPU::OP_AND, "AND Zeropage X" };
        instructionTable[0x2D] = {&CPU::AddressModeAbsolute, &CPU::OP_AND, "AND Abs" };
        instructionTable[0x3D] = {&CPU::AddressModeAbsoluteX, &CPU::OP_AND, "AND Abs X" };
        instructionTable[0x39] = {&CPU::AddressModeAbsoluteY, &CPU::OP_AND, "AND Abs Y" };
        instructionTable[0x21] = {&CPU::AddressModeIndirectX, &CPU::OP_AND, "AND Indirect X" };
        instructionTable[0x31] = {&CPU::AddressModeIndirectY, &CPU::OP_AND, "AND Indirect Y" };

        instructionTable[0x0A] = {&CPU::AddressModeAccumulator, &CPU::OP_ASL_ACC, "ASL Acc" };
        instructionTable[0x06] = {&CPU::AddressModeZeropage, &CPU::OP_ASL, "ASL Zeropage" };
        instructionTable[0x16] = {&CPU::AddressModeZeropageX, &CPU::OP_ASL, "ASL Zeropage X" };
        instructionTable[0x0E] = {&CPU::AddressModeAbsolute, &CPU::OP_ASL, "ASL Abs" };
        instructionTable[0x1E] = {&CPU::AddressModeAbsoluteX, &CPU::OP_ASL, "ASL Abs X" };

        instructionTable[0x61] = {&CPU::AddressModeIndirectX, &CPU::OP_ADC,
                                  "ADC Indirect X"};
        instructionTable[0x71] = {&CPU::AddressModeIndirectY, &CPU::OP_ADC,
                                  "ADC Indirect Y"};
        instructionTable[0x65] = {&CPU::AddressModeZeropage, &CPU::OP_ADC,
                                  "ADC Zeropage"};
        instructionTable[0x75] = {&CPU::AddressModeZeropageX, &CPU::OP_ADC,
                                  "ADC Zeropage Y"};
        instructionTable[0x6D] = {&CPU::AddressModeAbsolute, &CPU::OP_ADC,
                                  "ADC Abs"};
        instructionTable[0x7D] = {&CPU::AddressModeAbsoluteX, &CPU::OP_ADC,
                                  "ADC Abs X"};
        instructionTable[0x79] = {&CPU::AddressModeAbsoluteY, &CPU::OP_ADC,
                                  "ADC Abs Y"};
        

        instructionTable[0xEA] = {&CPU::AddressModeImplied, &CPU::OP_NOP, "NOP" };

    }

    void CPU::InvokeIRQ()
    {
        StackPush(regPC >> 8);
        StackPush(regPC & 0xFF);
        StackPush(regStatus & ~FLAG_B);
        regStatus.I = 1;
        regPC = Read16(IRQ_VECTOR);
    }

    void CPU::InvokeNMI()
    {
        StackPush(regPC >> 8);
        StackPush(regPC & 0xFF);
        StackPush(regStatus & ~FLAG_B);
        regStatus.I = 1;
        regPC = Read16(NMI_VECTOR);
        m_nmi = false;
    }

    std::uint16_t CPU::Read16(std::uint16_t addr)
    {
        std::uint16_t lowByte = Read(addr);
        std::uint16_t highByte = Read(addr + 1);
        return lowByte | (highByte << 8);
    }

    /// The operand is immediately following the op-code
    void CPU::AddressModeImmediate()
    {
        m_address = regPC++;
    }

    /// The address to the operand is the 2 bytes succeeding the op-code
    void CPU::AddressModeAbsolute()
    {
        m_address = Read16(regPC);
        regPC += 2;
    }

    /// The address to the operand is the 2 bytes succeeding the op-code + value of register X
    void CPU::AddressModeAbsoluteX()
    {
        m_address = Read16(regPC) + regX;
        regPC += 2;
    }

    /// The address to the operand is the 2 bytes succeeding the op-code + value of register Y
    void CPU::AddressModeAbsoluteY()
    {
        m_address = Read16(regPC) + regY;
        regPC += 2;
    }

    /// The address to the operand is the byte succeeding the op-code extended to 16bits
    void CPU::AddressModeZeropage()
    {
        m_address = Read(regPC++);
    }

    /// Zeropage X-Indexed
    /// The address to the operand is the byte succeeding the op-code + register X
    /// If the address gets larger than 0x100(255) the address will wrap and gets back to 0
    void CPU::AddressModeZeropageX()
    {
        std::uint8_t addr = ((Read(regPC++) + regX) % 0x100);
        m_address = addr;
    }

    /// Zeropage Y-Indexed
    /// The address to the operand is the byte succeeding the op-code + register Y
    /// If the address gets larger than 0x100(255) the address will wrap and gets back to 0
    void CPU::AddressModeZeropageY()
    {
        std::uint8_t addr = ((Read(regPC++) + regY) % 0x100);
        m_address = addr;
    }

    /// The two bytes that follow the op-code is an address which contains the LS-Byte of the real
    /// target address. The other byte is located in "target address + 1". Due to an error in the original
    /// design, if the target address is located on a page-boundary, the last byte of the address will be on
    /// 0xYY00
    void CPU::AddressModeIndirect()
    {
        std::uint16_t indirection = Read16(regPC);
        std::uint8_t low = Read(indirection);
        std::uint8_t high = Read((0xFF00 & indirection) | ((indirection + 1) % 0x100));
        m_address = low | (high << 8);
    }

    void CPU::AddressModeIndirectX()
    {
        // This address is used to index the zeropage
        std::uint8_t addr = ((Read(regPC++) + regX) % 0x100);
        std::uint8_t low = Read(addr);
        // Wrap if the address gets to 255
        std::uint8_t high = Read(addr + 1) % 0x100;
        m_address = low | (high << 8);
    }

    void CPU::AddressModeIndirectY()
    {
        // This address is used to index the zeropage
        std::uint8_t addr = ((Read(regPC++) + regY) % 0x100);
        std::uint8_t low = Read(addr);
        // Wrap if the address gets to 255
        std::uint8_t high = Read(addr + 1) % 0x100;
        // Add the contents of Y to get the final address
        m_address = (low | (high << 8)) + regY;

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
        Write(0x0100 | regSP--, value);
    }

    std::uint8_t CPU::StackPop()
    {
        return Read(0x0100 | ++regSP);
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
        else if (m_irq && !regStatus.I)
            InvokeIRQ();

        // Fetch
        std::uint8_t opcode = Read(regPC++);

        // Decode
        Instruction& instr = instructionTable[opcode];

        // TODO: This preprocessor statement might not work for everyone
#ifndef NDEBUG
        // Print the instruction name in debug mode
        std::cout << "0x" << std::hex << regPC - 1 << ": " << instr.name << std::endl;
#endif

        // Execute
        (this->*instr.addr)(); // Fetch the operand (if necessary)
        (this->*instr.op)();   // Execute the instruction
    }

    void CPU::Dump()
    {
        std::cout <<
                  "Register X: " << (unsigned int)regX << "\n" <<
                  "Register Y: " << (unsigned int)regY << "\n" <<
                  "Register A: " << (unsigned int)regA << "\n" <<
                  "Register SP: " << (unsigned int)regSP << "\n" <<
                  "Register PC: " << std::hex << regPC << "\n" <<
                  "Flags: " << std::bitset<8>(regStatus) << std::dec << "\n\n";
    }

    void CPU::Reset()
    {
        regPC = 0x0400;
        regStatus = 0x34;
        regX = 0;
        regY = 0;
        regA = 0;
        regSP = 0xFD;
        m_address = 0;
        m_nmi = false;
        m_irq = false;
        m_illegal_opcode = false;
    }

    void CPU::Run()
    {
        while(!m_illegal_opcode)
            Step();
    }

    void CPU::IllegalOP()
    {
        m_illegal_opcode = true;
    }

    // A + M + C -> A, C
    void CPU::OP_ADC()
    {
        std::uint8_t operand = Read(m_address);
        std::uint32_t result = regA + operand + regStatus.C;
        bool overflow = !((regA ^ operand) & 0x80) && ((regA ^ result) & 0x80);
        regStatus.Z = (result & 0xFF) == 0;
        regStatus.C = result > 0xFF;
        regStatus.V = overflow;
        regStatus.N = result & 0x80;
        regA = result & 0xFF;
    }
    
    void CPU::OP_BRK()
    {
        StackPush((regPC + 1) >> 8);
        StackPush((regPC + 1) & 0xFF);
        StackPush(regStatus | FLAG_B | FLAG_UNUSED);
        regStatus.I = 1;
        regPC = Read16(IRQ_VECTOR);
	}

	void CPU::OP_INC()
    {
        std::uint8_t operand = Read(m_address);
        operand++;
        Write(m_address, operand);
        regStatus.Z = operand == 0;
        regStatus.N = operand & 0x80;
    }

    void CPU::OP_DEC()
    {
        std::uint8_t operand = Read(m_address);
        operand--;
        Write(m_address, operand);
        regStatus.Z = operand == 0;
        regStatus.N = operand & 0x80;
    }

    void CPU::OP_INX()
    {
        regX++;
        regStatus.Z = regX == 0;
        regStatus.N = regX & 0x80;
    }

    void CPU::OP_INY()
    {
        regY++;
        regStatus.Z = regX == 0;
        regStatus.N = regX & 0x80;
    }

    void CPU::OP_DEY()
    {
        regY--;
        regStatus.Z = regY == 0;
        regStatus.N = regY & 0x80;
    }

    void CPU::OP_DEX()
    {
        regX--;
        regStatus.Z = regX == 0;
        regStatus.N = regX & 0x80;
    }

    void CPU::OP_NOP()
    {
        // No operation
	}

    void CPU::OP_LDY()
    {
        // Load index Y with memory
        std::uint8_t operand = Read(m_address);
        regY = operand;
        regStatus.Z = operand == 0;
        regStatus.N = operand & 0x80;
	}

    void CPU::OP_LDA()
    {
        // Load Accumulator
        std::uint8_t operand = Read(m_address);
        regA = operand;
        regStatus.Z = operand == 0;
        regStatus.N = operand & 0x80;
	}

    void CPU::OP_LDX()
    {
        // Load Accumulator
        std::uint8_t operand = Read(m_address);
        regX = operand;
        regStatus.Z = operand == 0;
        regStatus.N = operand & 0x80;
	}

    void CPU::OP_JMP()
    {
        regPC = m_address;
    }

    void CPU::OP_JSR()
    {
        std::uint16_t returnAddress = regPC - 1; // TODO: Should be just regPC since we increment in step()?
        StackPush(returnAddress >> 8); // Push PC high
        StackPush(returnAddress);            // Push PC low
        regPC = m_address;
    }

    void CPU::OP_BNE()
    {
        std::int8_t operand = Read(m_address);
        if(!regStatus.Z)
            regPC += operand;
    }

    void CPU::OP_BEQ()
    {
        std::int8_t operand = Read(m_address);
        if(regStatus.Z)
            regPC += operand;
    }

    void CPU::OP_BMI()
    {
        std::int8_t operand = Read(m_address);
        if(regStatus.N)
            regPC += operand;
    }

    void CPU::OP_BCC()
    {
        std::int8_t operand = Read(m_address);
        if(!regStatus.C)
            regPC += operand;
    }

    void CPU::OP_BCS()
    {
        std::int8_t operand = Read(m_address);
        if(regStatus.C)
            regPC += operand;
    }

    void CPU::OP_BPL()
    {
        std::int8_t operand = Read(m_address);
        if(!regStatus.N)
            regPC += operand;
    }

    void CPU::OP_BVC()
    {
        std::int8_t operand = Read(m_address);
        if(!regStatus.V)
            regPC += operand;
    }

    void CPU::OP_BVS()
    {
        std::int8_t operand = Read(m_address);
        if(regStatus.V)
            regPC += operand;
    }

    void CPU::OP_SEI()
    {
        regStatus = regStatus | FLAG_I;
    }
    void CPU::OP_CLI()
    {
        regStatus = regStatus & ~FLAG_I;
    }

    void CPU::OP_CLC()
    {
        regStatus = regStatus & ~FLAG_C;
    }

    void CPU::OP_CLD()
    {
        regStatus = regStatus & ~FLAG_D;
    }

    void CPU::OP_CLV()
    {
        regStatus = regStatus & ~FLAG_V;
    }

    void CPU::OP_BIT()
    {
        std::uint8_t operand = Read(m_address);
        regStatus.N = operand & 0x80;
        regStatus.V = operand & 0x40;
        regStatus.Z = (operand & regA) == 0;
    }

    void CPU::OP_EOR()
    {
        std::uint8_t operand = Read(m_address);
        regA ^= operand;
        regStatus.Z = regA == 0;
        regStatus.N = regA & 0x80;
    }

    void CPU::OP_AND()
    {
        std::uint8_t operand = Read(m_address);
        regA &= operand;
        regStatus.Z = regA == 0;
        regStatus.N = regA & 0x80;
    }

    void CPU::OP_ASL()
    {
        std::uint8_t operand = Read(m_address);
        regStatus.C = operand & 0x80;
        operand <<= 1;
        Write(m_address, operand);
        regStatus.Z = operand == 0;
        regStatus.N = operand & 0x80;
    }
    void CPU::OP_ASL_ACC()
    {
        regStatus.C = regA & 0x80;
        regA <<= 1;
        regStatus.Z = regA == 0;
        regStatus.N = regA & 0x80;
    }

    void CPU::OP_LSR()
    {
        std::uint8_t operand = Read(m_address);
        regStatus.C = operand & 1;
        operand >>= 1;
        Write(m_address, operand);
        regStatus.Z = operand == 0;
        regStatus.N = 0;
    }

    void CPU::OP_LSR_ACC()
    {
        regStatus.C = regA & 1;
        regA >>= 1;
        regStatus.Z = regA == 0;
        regStatus.N = 0;
    }

    void CPU::OP_ORA()
    {
        std::uint8_t operand = Read(m_address);
        regA |= operand;
        regStatus.Z = regA == 0;
        regStatus.N = regA & 0x80;
    }

    void CPU::OP_STY()
    {
        Write(m_address, regY);
    }

    void CPU::OP_STA()
    {
        Write(m_address, regA);
    }

    void CPU::OP_STX()
    {
        Write(m_address, regX);
    }

    void CPU::OP_PHA()
    {
        StackPush(regA);
    }

    void CPU::OP_PHP()
    {
        StackPush(regStatus | FLAG_B | FLAG_UNUSED);
    }

    void CPU::OP_PLA()
    {
        regA = StackPop();
        regStatus.Z = regA == 0;
        regStatus.N = regA & 0x80;
    }

    void CPU::OP_PLP()
    {
        regStatus = StackPop();
    }

    void CPU::OP_ROL()
    {
        std::uint32_t operand = Read(m_address);
        operand <<= 1;
        operand = regStatus.C ? operand | 1 : operand & ~1;
        regStatus.C = operand & 0x0100;
        Write(m_address, operand & 0xFF);
        regStatus.Z = (operand & 0xFF) == 0;
        regStatus.N = (operand & 0xFF) & 0x80;
    }

    void CPU::OP_ROL_ACC()
    {
        std::uint32_t operand = regA;
        operand <<= 1;
        operand = regStatus.C ? operand | 1 : operand & ~1;
        regStatus.C = operand & 0x0100;
        Write(m_address, operand & 0xFF);
        regStatus.Z = (operand & 0xFF) == 0;
        regStatus.N = (operand & 0xFF) & 0x80;
    }
    void CPU::OP_ROR_ACC()
    {
        std::uint32_t operand = regA;
        operand = regStatus.C ? operand | 0x0100 : operand & ~0x0100;
        regStatus.C = operand & 1;
        operand >>= 1;
        Write(m_address, operand & 0xFF);
        regStatus.Z = (operand & 0xFF) == 0;
        regStatus.N = (operand & 0xFF) & 0x80;
    }

    void CPU::OP_ROR()
    {
        std::uint32_t operand = Read(m_address);
        operand = regStatus.C ? operand | 0x0100 : operand & ~0x0100;
        regStatus.C = operand & 1;
        operand >>= 1;
        Write(m_address, operand & 0xFF);
        regStatus.Z = (operand & 0xFF) == 0;
        regStatus.N = (operand & 0xFF) & 0x80;
    }

    void CPU::OP_RTI()
    {
        regStatus = StackPop();
        regPC = StackPop() | (StackPop() << 8);
    }

    void CPU::OP_RTS()
    {
        regPC = (StackPop() | (StackPop() << 8)) + 1;
    }

    // A - M - C -> A
    void CPU::OP_SBC()
    {
        std::uint8_t operand = Read(m_address);
        std::uint32_t result = regA - operand - !regStatus.C;
        bool overflow = ((regA ^ result) & 0x80) && ((regA ^ operand) & 0x80);
        regStatus.Z = (result & 0xFF) == 0;
        regStatus.C = result < 0x0100;
        regStatus.V = overflow;
        regStatus.N = result & 0x80;
        regA = result & 0xFF;
    }

    void CPU::OP_SEC()
    {
        regStatus.C = 1;
    }

    void CPU::OP_SED()
    {
        regStatus.D = 1;
    }

    void CPU::OP_TAX()
    {
        regX = regA;
        regStatus.Z = regX == 0;
        regStatus.N = regX & 0x80;
    }

    void CPU::OP_TSX()
    {
        regX = regSP;
        regStatus.Z = regX == 0;
        regStatus.N = regX & 0x80;
    }

    void CPU::OP_TYA()
    {
        regA = regY;
        regStatus.Z = regA == 0;
        regStatus.N = regA & 0x80;
    }

    void CPU::OP_TXS()
    {
        regSP = regX;
    }

    void CPU::OP_TXA()
    {
        regA = regX;
        regStatus.Z = regA == 0;
        regStatus.N = regA & 0x80;
    }

    void CPU::OP_TAY()
    {
        regY = regA;
        regStatus.Z = regY == 0;
        regStatus.N = regY & 0x80;
    }

    // A - M
    void CPU::OP_CMP()
    {
        std::uint8_t operand = Read(m_address);
        std::uint32_t result = regA - operand;
        regStatus.Z = regA == operand;
        regStatus.N = result & 0x80;
        regStatus.C = result < 0x0100;
    }
    // X - M
    void CPU::OP_CPX()
    {
        std::uint8_t operand = Read(m_address);
        std::uint32_t result = regX - operand;
        regStatus.Z = regX == operand;
        regStatus.N = result & 0x80;
        regStatus.C = result < 0x0100;
    }

    // Y - M
    void CPU::OP_CPY()
    {
        std::uint8_t operand = Read(m_address);
        std::uint32_t result = regY - operand;
        regStatus.Z = regY == operand;
        regStatus.N = result & 0x80;
        regStatus.C = result < 0x0100;
    }

}
