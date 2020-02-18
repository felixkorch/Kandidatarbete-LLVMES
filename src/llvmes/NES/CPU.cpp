#include "llvmes/NES/CPU.h"
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
        , irq(false)
        , nmi(false)
        , instructionTable(0xFF)
        , illegalOpcode(false)
        , address(0)
    {
        for(auto& it : instructionTable)
            it = {&CPU::addressModeImplied, &CPU::illegalOP };

        instructionTable[0xD0] = {&CPU::addressModeImmediate, &CPU::opBNE, "BNE" };
        instructionTable[0xB0] = {&CPU::addressModeImmediate, &CPU::opBEQ, "BEQ" };
        instructionTable[0x30] = {&CPU::addressModeImmediate, &CPU::opBMI, "BMI" };
        instructionTable[0x90] = {&CPU::addressModeImmediate, &CPU::opBCC, "BCC" };
        instructionTable[0xB0] = {&CPU::addressModeImmediate, &CPU::opBCS, "BCS" };
        instructionTable[0x10] = {&CPU::addressModeImmediate, &CPU::opBPL, "BPL" };
        instructionTable[0x50] = {&CPU::addressModeImmediate, &CPU::opBVC, "BVC" };
        instructionTable[0x70] = {&CPU::addressModeImmediate, &CPU::opBVS, "BVS" };

        instructionTable[0xE8] = {&CPU::addressModeImplied, &CPU::opINX, "INX" };
        instructionTable[0xC8] = {&CPU::addressModeImplied, &CPU::opINY, "INY" };
        instructionTable[0x88] = {&CPU::addressModeImplied, &CPU::opDEY, "DEY" };
        instructionTable[0xCA] = {&CPU::addressModeImplied, &CPU::opDEX, "DEX" };

        instructionTable[0xE6] = {&CPU::addressModeZeropage, &CPU::opINC, "INC" };
        instructionTable[0xF6] = {&CPU::addressModeZeropageX, &CPU::opINC, "INC" };
        instructionTable[0xEE] = {&CPU::addressModeAbsolute, &CPU::opINC, "INC" };
        instructionTable[0xFE] = {&CPU::addressModeAbsoluteX, &CPU::opINC, "INC" };

        instructionTable[0x4C] = {&CPU::addressModeAbsolute, &CPU::opJMP, "JMP Abs" };
        instructionTable[0x6C] = {&CPU::addressModeIndirect, &CPU::opJMP, "JMP Indirect" };
        instructionTable[0x20] = {&CPU::addressModeAbsolute, &CPU::opJSR, "JSR" };

        instructionTable[0x24] = {&CPU::addressModeZeropage, &CPU::opBIT, "BIT Zeropage" };
        instructionTable[0x2C] = {&CPU::addressModeAbsolute, &CPU::opBIT, "BIT Abs" };

        instructionTable[0x00] = {&CPU::addressModeImplied, &CPU::opBRK, "BRK" };

        instructionTable[0x69] = {&CPU::addressModeImmediate, &CPU::opADC, "ADC Imm" };

        instructionTable[0xC9] = {&CPU::addressModeImmediate, &CPU::opCMP, "CMP Imm" };
        instructionTable[0xC5] = {&CPU::addressModeZeropage, &CPU::opCMP, "CMP Zeropage" };
        instructionTable[0xD5] = {&CPU::addressModeZeropageX, &CPU::opCMP, "CMP Zeropage X" };
        instructionTable[0xCD] = {&CPU::addressModeAbsolute, &CPU::opCMP, "CMP Abs" };
        instructionTable[0xDD] = {&CPU::addressModeAbsoluteX, &CPU::opCMP, "CMP Abs X" };
        instructionTable[0xD9] = {&CPU::addressModeAbsoluteY, &CPU::opCMP, "CMP Abs Y" };
        instructionTable[0xC1] = {&CPU::addressModeIndirectX, &CPU::opCMP, "CMP Indirect X" };
        instructionTable[0xD1] = {&CPU::addressModeIndirectY, &CPU::opCMP, "CMP Indirect Y" };

        instructionTable[0xE0] = {&CPU::addressModeImmediate, &CPU::opCPX, "CPX" };
        instructionTable[0xE4] = {&CPU::addressModeZeropage, &CPU::opCPX, "CPX" };
        instructionTable[0xEC] = {&CPU::addressModeAbsolute, &CPU::opCPX, "CPX" };

        instructionTable[0xC0] = {&CPU::addressModeImmediate, &CPU::opCPY, "CPY" };
        instructionTable[0xC4] = {&CPU::addressModeZeropage, &CPU::opCPY, "CPY" };
        instructionTable[0xCC] = {&CPU::addressModeAbsolute, &CPU::opCPY, "CPY" };

        instructionTable[0xC6] = {&CPU::addressModeZeropage, &CPU::opDEC, "DEC" };
        instructionTable[0xD6] = {&CPU::addressModeZeropageX, &CPU::opDEC, "DEC" };
        instructionTable[0xCE] = {&CPU::addressModeAbsolute, &CPU::opDEC, "DEC" };
        instructionTable[0xDE] = {&CPU::addressModeAbsoluteX, &CPU::opDEC, "DEC" };

        instructionTable[0x49] = {&CPU::addressModeImmediate, &CPU::opEOR, "EOR Imm" };
        instructionTable[0x45] = {&CPU::addressModeZeropage, &CPU::opEOR, "EOR Zeropage" };
        instructionTable[0x55] = {&CPU::addressModeZeropageX, &CPU::opEOR, "EOR Zeropage X" };
        instructionTable[0x4D] = {&CPU::addressModeAbsolute, &CPU::opEOR, "EOR Abs" };
        instructionTable[0x5D] = {&CPU::addressModeAbsoluteX, &CPU::opEOR, "EOR Abs X" };
        instructionTable[0x59] = {&CPU::addressModeAbsoluteY, &CPU::opEOR, "EOR Abs Y" };
        instructionTable[0x41] = {&CPU::addressModeIndirectX, &CPU::opEOR, "EOR Indirect X" };
        instructionTable[0x51] = {&CPU::addressModeIndirectY, &CPU::opEOR, "EOR Indirect Y" };

        instructionTable[0xA9] = {&CPU::addressModeImmediate, &CPU::opLDA, "LDA Imm" };
        instructionTable[0xA5] = {&CPU::addressModeZeropage, &CPU::opLDA, "LDA Zeropage" };
        instructionTable[0xB5] = {&CPU::addressModeZeropageX, &CPU::opLDA, "LDA Zeropage X" };
        instructionTable[0xA1] = {&CPU::addressModeIndirectX, &CPU::opLDA, "LDA Indirect X" };
        instructionTable[0xB1] = {&CPU::addressModeIndirectY, &CPU::opLDA, "LDA Indirect Y" };
        instructionTable[0xAD] = {&CPU::addressModeAbsolute, &CPU::opLDA, "LDA Abs" };
        instructionTable[0xBD] = {&CPU::addressModeAbsoluteX, &CPU::opLDA, "LDA Abs X" };
        instructionTable[0xB9] = {&CPU::addressModeAbsoluteY, &CPU::opLDA, "LDA Abs Y" };

        instructionTable[0xA2] = {&CPU::addressModeImmediate, &CPU::opLDX, "LDX Imm" };
        instructionTable[0xA6] = {&CPU::addressModeZeropage, &CPU::opLDX, "LDX Zeropage" };
        instructionTable[0xB6] = {&CPU::addressModeZeropageY, &CPU::opLDX, "LDX Zeropage Y" };
        instructionTable[0xAE] = {&CPU::addressModeAbsolute, &CPU::opLDX, "LDX Abs" };
        instructionTable[0xBE] = {&CPU::addressModeAbsoluteY, &CPU::opLDX, "LDX Abs Y" };

        instructionTable[0xA0] = {&CPU::addressModeImmediate, &CPU::opLDY, "LDY Imm" };
        instructionTable[0xA4] = {&CPU::addressModeZeropage, &CPU::opLDY, "LDY Zeropage" };
        instructionTable[0xB4] = {&CPU::addressModeZeropageX, &CPU::opLDY, "LDY Zeropage X" };
        instructionTable[0xAC] = {&CPU::addressModeAbsolute, &CPU::opLDY, "LDY Abs" };
        instructionTable[0xBC] = {&CPU::addressModeAbsoluteX, &CPU::opLDY, "LDY Abs X" };

        instructionTable[0x4A] = {&CPU::addressModeAccumulator, &CPU::opLSRAcc, "LSR Acc" };
        instructionTable[0x46] = {&CPU::addressModeZeropage, &CPU::opLSR, "LSR Zeropage" };
        instructionTable[0x56] = {&CPU::addressModeZeropageX, &CPU::opLSR, "LSR Zeropage X" };
        instructionTable[0x4E] = {&CPU::addressModeAbsolute, &CPU::opLSR, "LSR Abs" };
        instructionTable[0x5E] = {&CPU::addressModeAbsoluteX, &CPU::opLSR, "LSR Abs X" };

        instructionTable[0x09] = {&CPU::addressModeImmediate, &CPU::opORA, "ORA Imm" };
        instructionTable[0x05] = {&CPU::addressModeZeropage, &CPU::opORA, "ORA Zeropage" };
        instructionTable[0x15] = {&CPU::addressModeZeropageX, &CPU::opORA, "ORA Zeropage X" };
        instructionTable[0x0D] = {&CPU::addressModeAbsolute, &CPU::opORA, "ORA Abs" };
        instructionTable[0x1D] = {&CPU::addressModeAbsoluteX, &CPU::opORA, "ORA Abs X" };
        instructionTable[0x19] = {&CPU::addressModeAbsoluteY, &CPU::opORA, "ORA Abs Y" };
        instructionTable[0x01] = {&CPU::addressModeIndirectX, &CPU::opORA, "ORA Indirect X" };
        instructionTable[0x11] = {&CPU::addressModeIndirectY, &CPU::opORA, "ORA Indirect Y" };

        instructionTable[0x48] = {&CPU::addressModeImplied, &CPU::opPHA, "PHA" };
        instructionTable[0x08] = {&CPU::addressModeImplied, &CPU::opPHP, "PHP" };
        instructionTable[0x68] = {&CPU::addressModeImplied, &CPU::opPLA, "PLA" };
        instructionTable[0x28] = {&CPU::addressModeImplied, &CPU::opPLP, "PLP" };

        instructionTable[0x2A] = {&CPU::addressModeImplied, &CPU::opROLAcc, "ROL Acc" };
        instructionTable[0x26] = {&CPU::addressModeZeropage, &CPU::opROL, "ROL Zeropage" };
        instructionTable[0x36] = {&CPU::addressModeZeropageX, &CPU::opROL, "ROL Zeropage X" };
        instructionTable[0x2E] = {&CPU::addressModeAbsolute, &CPU::opROL, "ROL Abs" };
        instructionTable[0x3E] = {&CPU::addressModeAbsoluteX, &CPU::opROL, "ROL Abs X" };

        instructionTable[0x6A] = {&CPU::addressModeImplied, &CPU::opRORAcc, "ROR Acc" };
        instructionTable[0x66] = {&CPU::addressModeZeropage, &CPU::opROR, "ROR Zeropage" };
        instructionTable[0x76] = {&CPU::addressModeZeropageX, &CPU::opROR, "ROR Zeropage X" };
        instructionTable[0x6E] = {&CPU::addressModeAbsolute, &CPU::opROR, "ROR Abs" };
        instructionTable[0x7E] = {&CPU::addressModeAbsoluteX, &CPU::opROR, "ROR Abs X" };

        instructionTable[0x40] = {&CPU::addressModeImplied, &CPU::opRTI, "RTI" };
        instructionTable[0x60] = {&CPU::addressModeImplied, &CPU::opRTS, "RTS" };

        instructionTable[0xE9] = {&CPU::addressModeImmediate, &CPU::opSBC, "SBC Imm" };
        instructionTable[0xE5] = {&CPU::addressModeZeropage, &CPU::opSBC, "SBC Zeropage" };
        instructionTable[0xF5] = {&CPU::addressModeZeropageX, &CPU::opSBC, "SBC ZeropageX" };
        instructionTable[0xED] = {&CPU::addressModeAbsolute, &CPU::opSBC, "SBC Abs" };
        instructionTable[0xFD] = {&CPU::addressModeAbsoluteX, &CPU::opSBC, "SBC Abs X" };
        instructionTable[0xF9] = {&CPU::addressModeAbsoluteY, &CPU::opSBC, "SBC Abs Y" };
        instructionTable[0xE1] = {&CPU::addressModeIndirectX, &CPU::opSBC, "SBC Indirect X" };
        instructionTable[0xF1] = {&CPU::addressModeIndirectY, &CPU::opSBC, "SBC Indirect Y" };

        instructionTable[0x38] = {&CPU::addressModeImplied, &CPU::opSEC, "SEC" };
        instructionTable[0xF8] = {&CPU::addressModeImplied, &CPU::opSED, "SED" };
        instructionTable[0x78] = {&CPU::addressModeImplied, &CPU::opSEI, "SEI" };

        instructionTable[0x18] = {&CPU::addressModeImplied, &CPU::opCLC, "CLC" };
        instructionTable[0xD8] = {&CPU::addressModeImplied, &CPU::opCLD, "CLD" };
        instructionTable[0x58] = {&CPU::addressModeImplied, &CPU::opCLI, "CLI" };
        instructionTable[0xB8] = {&CPU::addressModeImplied, &CPU::opCLV, "CLV" };

        instructionTable[0x85] = {&CPU::addressModeZeropage, &CPU::opSTA, "STA Zeropage" };
        instructionTable[0x95] = {&CPU::addressModeZeropageX, &CPU::opSTA, "STA Zeropage X" };
        instructionTable[0x8D] = {&CPU::addressModeAbsolute, &CPU::opSTA, "STA Abs" };
        instructionTable[0x9D] = {&CPU::addressModeAbsoluteX, &CPU::opSTA, "STA Abs X" };
        instructionTable[0x99] = {&CPU::addressModeAbsoluteY, &CPU::opSTA, "STA Abs Y" };
        instructionTable[0x81] = {&CPU::addressModeIndirectX, &CPU::opSTA, "STA Indirect X" };
        instructionTable[0x91] = {&CPU::addressModeIndirectY, &CPU::opSTA, "STA Indirect Y" };

        instructionTable[0x86] = {&CPU::addressModeZeropage, &CPU::opSTX, "STX Zeropage" };
        instructionTable[0x96] = {&CPU::addressModeZeropageY, &CPU::opSTX, "STX Zeropage Y" };
        instructionTable[0x8E] = {&CPU::addressModeAbsolute, &CPU::opSTX, "STX Abs" };

        instructionTable[0x84] = {&CPU::addressModeZeropage, &CPU::opSTY, "STY Zeropage" };
        instructionTable[0x94] = {&CPU::addressModeZeropageX, &CPU::opSTY, "STY Zeropage X" };
        instructionTable[0x8C] = {&CPU::addressModeAbsolute, &CPU::opSTY, "STY Abs" };

        instructionTable[0xAA] = {&CPU::addressModeImplied, &CPU::opTAX, "TAX" };
        instructionTable[0xA8] = {&CPU::addressModeImplied, &CPU::opTAY, "TAY" };
        instructionTable[0xBA] = {&CPU::addressModeImplied, &CPU::opTSX, "TSX" };
        instructionTable[0x8A] = {&CPU::addressModeImplied, &CPU::opTXA, "TXA" };
        instructionTable[0x9A] = {&CPU::addressModeImplied, &CPU::opTXS, "TXS" };
        instructionTable[0x98] = {&CPU::addressModeImplied, &CPU::opTYA, "TYA" };

        instructionTable[0x29] = {&CPU::addressModeImmediate, &CPU::opAND, "AND Imm" };
        instructionTable[0x25] = {&CPU::addressModeZeropage, &CPU::opAND, "AND Zeropage" };
        instructionTable[0x35] = {&CPU::addressModeZeropageX, &CPU::opAND, "AND Zeropage X" };
        instructionTable[0x2D] = {&CPU::addressModeAbsolute, &CPU::opAND, "AND Abs" };
        instructionTable[0x3D] = {&CPU::addressModeAbsoluteX, &CPU::opAND, "AND Abs X" };
        instructionTable[0x39] = {&CPU::addressModeAbsoluteY, &CPU::opAND, "AND Abs Y" };
        instructionTable[0x21] = {&CPU::addressModeIndirectX, &CPU::opAND, "AND Indirect X" };
        instructionTable[0x31] = {&CPU::addressModeIndirectY, &CPU::opAND, "AND Indirect Y" };

        instructionTable[0x0A] = {&CPU::addressModeAccumulator, &CPU::opASLAcc, "ASL Acc" };
        instructionTable[0x06] = {&CPU::addressModeZeropage, &CPU::opASL, "ASL Zeropage" };
        instructionTable[0x16] = {&CPU::addressModeZeropageX, &CPU::opASL, "ASL Zeropage X" };
        instructionTable[0x0E] = {&CPU::addressModeAbsolute, &CPU::opASL, "ASL Abs" };
        instructionTable[0x1E] = {&CPU::addressModeAbsoluteX, &CPU::opASL, "ASL Abs X" };

        instructionTable[0x61] = {&CPU::addressModeIndirectX, &CPU::opADC,
                                  "ADC Indirect X"};
        instructionTable[0x71] = {&CPU::addressModeIndirectY, &CPU::opADC,
                                  "ADC Indirect Y"};
        instructionTable[0x65] = {&CPU::addressModeZeropage, &CPU::opADC,
                                  "ADC Zeropage"};
        instructionTable[0x75] = {&CPU::addressModeZeropageX, &CPU::opADC,
                                  "ADC Zeropage Y"};
        instructionTable[0x6D] = {&CPU::addressModeAbsolute, &CPU::opADC,
                                  "ADC Abs"};
        instructionTable[0x7D] = {&CPU::addressModeAbsoluteX, &CPU::opADC,
                                  "ADC Abs X"};
        instructionTable[0x79] = {&CPU::addressModeAbsoluteY, &CPU::opADC,
                                  "ADC Abs Y"};
        

        instructionTable[0xEA] = {&CPU::addressModeImplied, &CPU::opNOP, "NOP" };

    }

    void CPU::invokeIRQ()
    {
        stackPush(regPC >> 8);
        stackPush(regPC & 0xFF);
        stackPush(regStatus & ~FLAG_B);
        regStatus.I = 1;
        regPC = read16(IRQ_VECTOR);
    }

    void CPU::invokeNMI()
    {
        stackPush(regPC >> 8);
        stackPush(regPC & 0xFF);
        stackPush(regStatus & ~FLAG_B);
        regStatus.I = 1;
        regPC = read16(NMI_VECTOR);
        nmi = false;
    }

    std::uint16_t CPU::read16(std::uint16_t addr)
    {
        std::uint16_t lowByte = read(addr);
        std::uint16_t highByte = read(addr + 1);
        return lowByte | (highByte << 8);
    }

    /// The operand is immediately following the op-code
    void CPU::addressModeImmediate()
    {
        address = regPC++;
    }

    /// The address to the operand is the 2 bytes succeeding the op-code
    void CPU::addressModeAbsolute()
    {
        address = read16(regPC);
        regPC += 2;
    }

    /// The address to the operand is the 2 bytes succeeding the op-code + value of register X
    void CPU::addressModeAbsoluteX()
    {
        address = read16(regPC) + regX;
        regPC += 2;
    }

    /// The address to the operand is the 2 bytes succeeding the op-code + value of register Y
    void CPU::addressModeAbsoluteY()
    {
        address = read16(regPC) + regY;
        regPC += 2;
    }

    /// The address to the operand is the byte succeeding the op-code extended to 16bits
    void CPU::addressModeZeropage()
    {
        address = read(regPC++);
    }

    /// Zeropage X-Indexed
    /// The address to the operand is the byte succeeding the op-code + register X
    /// If the address gets larger than 0x100(255) the address will wrap and gets back to 0
    void CPU::addressModeZeropageX()
    {
        std::uint8_t addr = ((read(regPC++) + regX) % 0x100);
        address = addr;
    }

    /// Zeropage Y-Indexed
    /// The address to the operand is the byte succeeding the op-code + register Y
    /// If the address gets larger than 0x100(255) the address will wrap and gets back to 0
    void CPU::addressModeZeropageY()
    {
        std::uint8_t addr = ((read(regPC++) + regY) % 0x100);
        address = addr;
    }

    /// The two bytes that follow the op-code is an address which contains the LS-Byte of the real
    /// target address. The other byte is located in "target address + 1". Due to an error in the original
    /// design, if the target address is located on a page-boundary, the last byte of the address will be on
    /// 0xYY00
    void CPU::addressModeIndirect()
    {
        std::uint16_t indirection = read16(regPC);
        std::uint8_t low = read(indirection);
        std::uint8_t high = read((0xFF00 & indirection) | ((indirection + 1) % 0x100));
        address = low | (high << 8);
    }

    void CPU::addressModeIndirectX()
    {
        // This address is used to index the zeropage
        std::uint8_t addr = ((read(regPC++) + regX) % 0x100);
        std::uint8_t low = read(addr);
        // Wrap if the address gets to 255
        std::uint8_t high = read(addr + 1) % 0x100;
        address = low | (high << 8);
    }

    void CPU::addressModeIndirectY()
    {
        // This address is used to index the zeropage
        std::uint8_t addr = ((read(regPC++) + regY) % 0x100);
        std::uint8_t low = read(addr);
        // Wrap if the address gets to 255
        std::uint8_t high = read(addr + 1) % 0x100;
        // Add the contents of Y to get the final address
        address = (low | (high << 8)) + regY;

    }

    void CPU::addressModeImplied()
    {
        // Simply means the instruction doesn't need an operand
    }

    void CPU::addressModeAccumulator()
    {
        // The operand is the contents of the accumulator(regA)
    }

    void CPU::stackPush(std::uint8_t value)
    {
        write(0x0100 | regSP--, value);
    }

    std::uint8_t CPU::stackPop()
    {
        return read(0x0100 | ++regSP);
    }

    void CPU::setNMI()
    {
        nmi = true;
    }

    void CPU::setIRQ()
    {
        irq = true;
    }

	void CPU::step()
    {
        // Interrupt handling
        if (nmi)
            invokeNMI();
        else if (irq && !regStatus.I)
            invokeIRQ();

        // Fetch
        std::uint8_t opcode = read(regPC++);

        // Decode
        Instruction& instr = instructionTable[opcode];

        // TODO: This preprocessor statement might not work for everyone
#ifndef NDEBUG
        // Print the instruction name in debug mode
        std::cout << instr.name << std::endl;
#endif

        // Execute
        (this->*instr.addr)(); // Fetch the operand (if necessary)
        (this->*instr.op)();   // Execute the instruction
    }

    void CPU::dump()
    {
        std::cout <<
                  "Register X: " << (unsigned int)regX << "\n" <<
                  "Register Y: " << (unsigned int)regY << "\n" <<
                  "Register A: " << (unsigned int)regA << "\n" <<
                  "Register SP: " << (unsigned int)regSP << "\n" <<
                  "Register PC: " << std::hex << regPC << "\n" <<
                  "Flags: " << std::bitset<8>(regStatus) << std::dec << "\n\n";
    }

    void CPU::reset()
    {
        regPC = read16(RESET_VECTOR);
    }

    void CPU::run()
    {
        while(!illegalOpcode)
            step();
    }

    void CPU::illegalOP()
    {
        illegalOpcode = true;
    }

    // A + M + C -> A, C
    void CPU::opADC()
    {
        std::uint8_t operand = read(address);
        std::uint32_t result = regA + operand + regStatus.C;
        bool overflow = !((regA ^ operand) & 0x80) && ((regA ^ result) & 0x80);
        regStatus.Z = (result & 0xFF) == 0;
        regStatus.C = result > 0xFF;
        regStatus.V = overflow;
        regStatus.N = result & 0x80;
        regA = result & 0xFF;
    }
    
    void CPU::opBRK()
    {
        stackPush(regPC >> 8);
        stackPush(regPC & 0xFF);
        stackPush(regStatus | FLAG_B);
        regStatus.I = 1;
        regPC = read16(IRQ_VECTOR);
	}

	void CPU::opINC()
    {
        std::uint8_t operand = read(address);
        operand++;
        write(address, operand);
        regStatus.Z = operand == 0;
        regStatus.N = operand & 0x80;
    }

    void CPU::opDEC()
    {
        std::uint8_t operand = read(address);
        operand--;
        write(address, operand);
        regStatus.Z = operand == 0;
        regStatus.N = operand & 0x80;
    }

    void CPU::opINX()
    {
        regX++;
        regStatus.Z = regX == 0;
        regStatus.N = regX & 0x80;
    }

    void CPU::opINY()
    {
        regY++;
        regStatus.Z = regX == 0;
        regStatus.N = regX & 0x80;
    }

    void CPU::opDEY()
    {
        regY--;
        regStatus.Z = regY == 0;
        regStatus.N = regY & 0x80;
    }

    void CPU::opDEX()
    {
        regX--;
        regStatus.Z = regX == 0;
        regStatus.N = regX & 0x80;
    }

    void CPU::opNOP()
    {
        // No operation
	}

    void CPU::opLDY()
    {
        // Load index Y with memory
        std::uint8_t operand = read(address);
        regY = operand;
        regStatus.Z = operand == 0;
        regStatus.N = operand & 0x80;
	}

    void CPU::opLDA()
    {
        // Load Accumulator
        std::uint8_t operand = read(address);
        regA = operand;
        regStatus.Z = operand == 0;
        regStatus.N = operand & 0x80;
	}

    void CPU::opLDX()
    {
        // Load Accumulator
        std::uint8_t operand = read(address);
        regX = operand;
        regStatus.Z = operand == 0;
        regStatus.N = operand & 0x80;
	}

    void CPU::opJMP()
    {
        regPC = address;
    }

    void CPU::opJSR()
    {
        std::uint16_t returnAddress = regPC + 1;
        stackPush(returnAddress >> 8); // Push PC high
        stackPush(returnAddress);            // Push PC low
        regPC = read16(regPC);
    }

    void CPU::opBNE()
    {
        std::int8_t operand = read(address);
        if(!regStatus.Z)
            regPC += operand;
    }

    void CPU::opBEQ()
    {
        std::int8_t operand = read(address);
        if(regStatus.Z)
            regPC += operand;
    }

    void CPU::opBMI()
    {
        std::int8_t operand = read(address);
        if(regStatus.Z)
            regPC += operand;
    }

    void CPU::opBCC()
    {
        std::int8_t operand = read(address);
        if(!regStatus.C)
            regPC += operand;
    }

    void CPU::opBCS()
    {
        std::int8_t operand = read(address);
        if(regStatus.C)
            regPC += operand;
    }

    void CPU::opBPL()
    {
        std::int8_t operand = read(address);
        if(!regStatus.N)
            regPC += operand;
    }

    void CPU::opBVC()
    {
        std::int8_t operand = read(address);
        if(!regStatus.C)
            regPC += operand;
    }

    void CPU::opBVS()
    {
        std::int8_t operand = read(address);
        if(regStatus.C)
            regPC += operand;
    }

    void CPU::opSEI()
    {
        regStatus = regStatus | FLAG_I;
    }
    void CPU::opCLI()
    {
        regStatus = regStatus & ~FLAG_I;
    }

    void CPU::opCLC()
    {
        regStatus = regStatus & ~FLAG_C;
    }

    void CPU::opCLD()
    {
        regStatus = regStatus & ~FLAG_D;
    }

    void CPU::opCLV()
    {
        regStatus = regStatus & ~FLAG_V;
    }

    void CPU::opBIT()
    {
        std::uint8_t operand = read(address);
        regStatus.N = operand & 0x80;
        regStatus.V = operand & 0x40;
        regStatus.Z = (operand & regA) == 0;
    }

    void CPU::opEOR()
    {
        std::uint8_t operand = read(address);
        regA ^= operand;
        regStatus.Z = regA == 0;
        regStatus.N = regA & 0x80;
    }

    void CPU::opAND()
    {
        std::uint8_t operand = read(address);
        regA &= operand;
        regStatus.Z = regA == 0;
        regStatus.N = regA & 0x80;
    }

    void CPU::opASL()
    {
        std::uint8_t operand = read(address);
        regStatus.C = operand & 0x80;
        operand <<= 1;
        write(address, operand);
        regStatus.Z = operand == 0;
        regStatus.N = operand & 0x80;
    }
    void CPU::opASLAcc()
    {
        regStatus.C = regA & 0x80;
        regA <<= 1;
        regStatus.Z = regA == 0;
        regStatus.N = regA & 0x80;
    }

    void CPU::opLSR()
    {
        std::uint8_t operand = read(address);
        regStatus.C = operand & 1;
        operand >>= 1;
        write(address, operand);
        regStatus.Z = operand == 0;
        regStatus.N = 0;
    }

    void CPU::opLSRAcc()
    {
        regStatus.C = regA & 1;
        regA >>= 1;
        regStatus.Z = regA == 0;
        regStatus.N = 0;
    }

    void CPU::opORA()
    {
        std::uint8_t operand = read(address);
        regA |= operand;
        regStatus.Z = regA == 0;
        regStatus.N = regA & 0x80;
    }

    void CPU::opSTY()
    {
        write(address, regY);
    }

    void CPU::opSTA()
    {
        write(address, regA);
    }

    void CPU::opSTX()
    {
        write(address, regX);
    }

    void CPU::opPHA()
    {
        stackPush(regA);
    }

    void CPU::opPHP()
    {
        stackPush(regStatus | FLAG_B | FLAG_UNUSED);
    }

    void CPU::opPLA()
    {
        regA = stackPop();
        regStatus.Z = regA == 0;
        regStatus.N = regA & 0x80;
    }

    void CPU::opPLP()
    {
        regStatus = stackPop();
    }

    void CPU::opROL()
    {
        std::uint32_t operand = read(address);
        operand <<= 1;
        operand = regStatus.C ? operand | 1 : operand & ~1;
        regStatus.C = operand & 0x0100;
        write(address, operand & 0xFF);
        regStatus.Z = (operand & 0xFF) == 0;
        regStatus.N = (operand & 0xFF) & 0x80;
    }

    void CPU::opROLAcc()
    {
        std::uint32_t operand = regA;
        operand <<= 1;
        operand = regStatus.C ? operand | 1 : operand & ~1;
        regStatus.C = operand & 0x0100;
        write(address, operand & 0xFF);
        regStatus.Z = (operand & 0xFF) == 0;
        regStatus.N = (operand & 0xFF) & 0x80;
    }
    void CPU::opRORAcc()
    {
        std::uint32_t operand = regA;
        operand = regStatus.C ? operand | 0x0100 : operand & ~0x0100;
        regStatus.C = operand & 1;
        operand >>= 1;
        write(address, operand & 0xFF);
        regStatus.Z = (operand & 0xFF) == 0;
        regStatus.N = (operand & 0xFF) & 0x80;
    }

    void CPU::opROR()
    {
        std::uint32_t operand = read(address);
        operand = regStatus.C ? operand | 0x0100 : operand & ~0x0100;
        regStatus.C = operand & 1;
        operand >>= 1;
        write(address, operand & 0xFF);
        regStatus.Z = (operand & 0xFF) == 0;
        regStatus.N = (operand & 0xFF) & 0x80;
    }

    void CPU::opRTI()
    {
        regStatus = stackPop();
        regPC = stackPop() | (stackPop() << 8);
    }

    void CPU::opRTS()
    {
        regPC = (stackPop() | (stackPop() << 8)) + 1;
    }

    // A - M - C -> A
    void CPU::opSBC()
    {
        std::uint8_t operand = read(address);
        std::uint32_t result = regA - operand - !regStatus.C;
        bool overflow = ((regA ^ result) & 0x80) && ((regA ^ operand) & 0x80);
        regStatus.Z = (result & 0xFF) == 0;
        regStatus.C = result < 0x0100;
        regStatus.V = overflow;
        regStatus.N = result & 0x80;
        regA = result & 0xFF;
    }

    void CPU::opSEC()
    {
        regStatus.C = 1;
    }

    void CPU::opSED()
    {
        regStatus.D = 1;
    }

    void CPU::opTAX()
    {
        regX = regA;
        regStatus.Z = regX == 0;
        regStatus.N = regX & 0x80;
    }

    void CPU::opTSX()
    {
        regX = regSP;
        regStatus.Z = regX == 0;
        regStatus.N = regX & 0x80;
    }

    void CPU::opTYA()
    {
        regA = regY;
        regStatus.Z = regA == 0;
        regStatus.N = regA & 0x80;
    }

    void CPU::opTXS()
    {
        regSP = regX;
    }

    void CPU::opTXA()
    {
        regA = regX;
        regStatus.Z = regA == 0;
        regStatus.N = regA & 0x80;
    }

    void CPU::opTAY()
    {
        regY = regA;
        regStatus.Z = regY == 0;
        regStatus.N = regY & 0x80;
    }

    // A - M
    void CPU::opCMP()
    {
        std::uint8_t operand = read(address);
        std::uint32_t result = regA - operand;
        regStatus.Z = regA == operand;
        regStatus.N = result & 0x80;
        regStatus.C = result < 0x0100;
    }
    // X - M
    void CPU::opCPX()
    {
        std::uint8_t operand = read(address);
        std::uint32_t result = regX - operand;
        regStatus.Z = regX == operand;
        regStatus.N = result & 0x80;
        regStatus.C = result < 0x0100;
    }

    // Y - M
    void CPU::opCPY()
    {
        std::uint8_t operand = read(address);
        std::uint32_t result = regY - operand;
        regStatus.Z = regY == operand;
        regStatus.N = result & 0x80;
        regStatus.C = result < 0x0100;
    }

}