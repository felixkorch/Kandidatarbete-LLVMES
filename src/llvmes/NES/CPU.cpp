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
        , instructionTable(0xFF)
        , illegalOpcode(false)
        , address(0)
    {
        for(auto& it : instructionTable)
            it = {&CPU::addressModeImplied, &CPU::illegalOP };

        instructionTable[0xA0] = {&CPU::addressModeImmediate, &CPU::opLDY, "LDY Imm" };
        instructionTable[0xD0] = {&CPU::addressModeImmediate, &CPU::opBNE, "BNE" };
        instructionTable[0xA2] = {&CPU::addressModeImmediate, &CPU::opLDX, "LDX Imm" };
        instructionTable[0xE8] = {&CPU::addressModeImplied, &CPU::opINX, "INX" };
        instructionTable[0x88] = {&CPU::addressModeImplied, &CPU::opDEY, "DEY" };
        instructionTable[0x69] = {&CPU::addressModeImmediate, &CPU::opADC, "ADC Imm" };

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

        instructionTable[0x4A] = {&CPU::addressModeAbsoluteX, &CPU::opLSR, "LSR Acc" };
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

        instructionTable[0x2A] = {&CPU::addressModeImmediate, &CPU::opROL, "ROL Acc" };
        instructionTable[0x26] = {&CPU::addressModeZeropage, &CPU::opROL, "ROL Zeropage" };
        instructionTable[0x36] = {&CPU::addressModeZeropageX, &CPU::opROL, "ROL Zeropage X" };
        instructionTable[0x2E] = {&CPU::addressModeAbsolute, &CPU::opROL, "ROL Abs" };
        instructionTable[0x3E] = {&CPU::addressModeAbsoluteX, &CPU::opROL, "ROL Abs X" };

        instructionTable[0x6A] = {&CPU::addressModeImmediate, &CPU::opROR, "ROR Acc" };
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

        instructionTable[0xF0] = {&CPU::addressModeImmediate, &CPU::opBEQ, "BEQ" };
        instructionTable[0xEA] = {&CPU::addressModeImplied, &CPU::opNOP, "NOP" };
    }

    std::uint16_t CPU::read16(std::uint16_t addr) {
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

    }

    void CPU::stackPush(std::uint8_t value)
    {
        write(0x0100 | regSP--, value);
    }

    std::uint8_t CPU::stackPop()
    {
        return read(0x0100 | ++regSP);
    }

	void CPU::step()
    {
        // Fetch
        std::uint8_t opcode = read(regPC++);

        // Decode
        Instruction& instr = instructionTable[opcode];
#ifndef NDEBUG
        // Print the instruction name in debug mode
        std::cout << instr.name << std::endl;
#endif

        // Execute
        (this->*instr.addr)(); // Fetch the operand (if necessary)
        (this->*instr.op)();   // Execute the instruction
    }

    void CPU::illegalOP()
    {
        illegalOpcode = true;
    }

    void CPU::opADC()
    {
        
    }
    
    void CPU::opBRK()
    {
        // Force Break 
		regStatus.I = 1;
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

    void CPU::opJMPIndirect()
    {

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

    void CPU::opJMPAbsolute()
    {
        std::uint16_t lowByte = read(regPC);
        std::uint16_t highByte = read(regPC + 1);
        regPC = lowByte | (highByte << 8);
    }

    void CPU::opSEI()
    {
        regStatus = regStatus | FLAG_I;
    }
    void CPU::opCLI()
    {
        regStatus = regStatus & ~FLAG_I;
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

    void CPU::opLSR()
    {

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

    void CPU::opPHA()
    {

    }

    void CPU::opPHP() {

    }

    void CPU::opPLA()
    {

    }

    void CPU::opPLP()
    {

    }

    void CPU::opROL()
    {

    }

    void CPU::opROR()
    {

    }

    void CPU::opRTI()
    {

    }

    void CPU::opRTS()
    {

    }

    void CPU::opSBC()
    {

    }

    void CPU::opSEC()
    {
        regStatus.C = 1;
    }

    void CPU::opSED()
    {
        regStatus.D = 1;
    }

    void CPU::opSTA()
    {

    }

    void CPU::opSTX()
    {

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

}