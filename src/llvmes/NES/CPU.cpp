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
        , fetched(0)
    {
        for(auto& it : instructionTable)
            it = {&CPU::addressModeImplied, &CPU::illegalOP };

        instructionTable[0xA0] = {&CPU::addressModeImmediate, &CPU::opLDY, "LDY Imm" };
        instructionTable[0xD0] = {&CPU::addressModeImmediate, &CPU::opBNE, "BNE" };
        instructionTable[0xA2] = {&CPU::addressModeImmediate, &CPU::opLDX, "LDX Imm" };
        instructionTable[0xE8] = {&CPU::addressModeImplied, &CPU::opINX, "INX" };
        instructionTable[0x88] = {&CPU::addressModeImplied, &CPU::opDEY, "DEY" };
        instructionTable[0x69] = {&CPU::addressModeImmediate, &CPU::opADC, "ADC Imm" };
        instructionTable[0x78] = {&CPU::addressModeImplied, &CPU::opSEI, "SEI" };
        instructionTable[0xA9] = {&CPU::addressModeImmediate, &CPU::opLDA, "LDA Imm" };
        instructionTable[0xA5] = {&CPU::addressModeZeropage, &CPU::opLDA, "LDA Zeropage" };
        instructionTable[0xAD] = {&CPU::addressModeAbsolute, &CPU::opLDA, "LDA Abs" };
        instructionTable[0xBD] = {&CPU::addressModeAbsoluteX, &CPU::opLDA, "LDA Abs X" };
        instructionTable[0xB9] = {&CPU::addressModeAbsoluteY, &CPU::opLDA, "LDA Abs Y" };
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
        fetched = read(regPC++);
    }

    /// The address to the operand is the 2 bytes succeeding the op-code
    void CPU::addressModeAbsolute()
    {
        std::uint16_t addr = read16(regPC);
        fetched = read(addr);
        regPC += 2;
    }

    /// The address to the operand is the 2 bytes succeeding the op-code + value of register X
    void CPU::addressModeAbsoluteX()
    {
        std::uint16_t addr = read16(regPC) + regX;
        fetched = read(addr);
        regPC += 2;
    }

    /// The address to the operand is the 2 bytes succeeding the op-code + value of register Y
    void CPU::addressModeAbsoluteY()
    {
        std::uint16_t addr = read16(regPC) + regY;
        fetched = read(addr);
        regPC += 2;
    }

    /// The address to the operand is the byte succeeding the op-code extended to 16bits
    void CPU::addressModeZeropage()
    {
        std::uint16_t addr = read(regPC++);
        fetched = read(addr);
    }

    void CPU::addressModeImplied()
    {
        // Simply means the instruction doesn't need an operand
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
        regY = fetched;
        regStatus.Z = fetched == 0;
        regStatus.N = fetched & 0x80;
	}

    void CPU::opLDA()
    {
        // Load Accumulator
        regA = fetched;
        regStatus.Z = fetched == 0;
        regStatus.N = fetched & 0x80;
	}

    void CPU::opLDX()
    {
        // Load Accumulator
        regX = fetched;
        regStatus.Z = fetched == 0;
        regStatus.N = fetched & 0x80;
	}

    void CPU::opJMPIndirect()
    {

    }

    void CPU::opBNE()
    {
        std::int8_t operand = fetched;
        if(!regStatus.Z)
            regPC += operand;
    }

    void CPU::opBEQ()
    {
        std::int8_t operand = fetched;
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


}