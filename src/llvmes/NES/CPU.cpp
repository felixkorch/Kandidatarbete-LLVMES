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
        , illegalOpcode(false)
    {
        for(auto& it : instructionTable)
            it = { &CPU::getAddressImplied, &CPU::illegalOP };

        instructionTable[0xA0] = {&CPU::getAddressImmediate, &CPU::opLDY };
        instructionTable[0xD0] = {&CPU::getAddressImmediate, &CPU::opBNE };
        instructionTable[0xA2] = {&CPU::getAddressImmediate, &CPU::opLDX };
        instructionTable[0xE8] = {&CPU::getAddressImplied, &CPU::opINX };
        instructionTable[0x88] = {&CPU::getAddressImplied, &CPU::opDEY };
        instructionTable[0x69] = {&CPU::getAddressImmediate, &CPU::opADC };
        instructionTable[0x78] = {&CPU::getAddressImplied, &CPU::opSEI };
        instructionTable[0xA9] = {&CPU::getAddressImmediate, &CPU::opLDA };
        instructionTable[0xAD] = {&CPU::getAddressAbsolute, &CPU::opLDA };
        instructionTable[0xF0] = {&CPU::getAddressImmediate, &CPU::opBEQ };
        instructionTable[0xE8] = {&CPU::getAddressImplied, &CPU::opNOP };
    }

    std::uint16_t CPU::read16(std::uint16_t adr) {
        std::uint16_t lowByte = read(adr);
        std::uint16_t highByte = read(adr + 1);
        return lowByte | (highByte << 8);
    }

    std::uint16_t CPU::getAddressImmediate()
    {
        return regPC++;
    }

    std::uint16_t CPU::getAddressAbsolute() {
        regPC += 2;
        return read16(regPC - 2);
    }

	void CPU::step()
    {
        // Fetch
        auto opcode = read(regPC++);

        // Decode
        auto instr = instructionTable[opcode];

        // Execute
        instr.execute(this);
    }

    void CPU::illegalOP(std::uint16_t)
    {
        std::cout << "Illegal OP!" << std::endl;
        illegalOpcode = true;
    }

    void CPU::opADC(std::uint16_t adr)
    {
        
    }
    
    void CPU::opBRK(std::uint16_t adr)
    {
        // Force Break 
		regStatus.I = 1;
	}

    void CPU::opINX(std::uint16_t adr)
    {
        regX++;
        regStatus.Z = regX == 0;
        regStatus.N = regX & 0x80;
    }

    void CPU::opINY(std::uint16_t adr)
    {
        regY++;
        regStatus.Z = regX == 0;
        regStatus.N = regX & 0x80;
    }

    void CPU::opDEY(std::uint16_t adr)
    {
        regY--;
        regStatus.Z = regY == 0;
        regStatus.N = regY & 0x80;
    }

    void CPU::opDEX(std::uint16_t adr)
    {
        regX--;
        regStatus.Z = regX == 0;
        regStatus.N = regX & 0x80;
    }

    void CPU::opNOP(std::uint16_t adr)
    {
        // No operation
	}

    void CPU::opLDY(std::uint16_t adr)
    {
        // Load index Y with memory
        std::uint8_t operand = read(adr);
        regY = operand;
        regStatus.Z = operand == 0;
        regStatus.N = operand & 0x80;
	}

    void CPU::opLDA(std::uint16_t adr)
    {
        // Load Accumulator
        std::uint8_t operand = read(adr);
        regA = operand;
        regStatus.Z = operand == 0;
        regStatus.N = operand & 0x80;
	}

    void CPU::opLDX(std::uint16_t adr)
    {
        // Load Accumulator
        std::uint8_t operand = read(adr);
        regX = operand;
        regStatus.Z = operand == 0;
        regStatus.N = operand & 0x80;
	}

    void CPU::opJMPIndirect(std::uint16_t adr)
    {

    }

    void CPU::opBNE(std::uint16_t adr)
    {
        std::int8_t operand = read(adr);
        if(!regStatus.Z)
            regPC += operand;
    }

    void CPU::opBEQ(std::uint16_t adr)
    {
        std::int8_t operand = read(adr);
        if(regStatus.Z)
            regPC += operand;
    }

    void CPU::opJMPAbsolute(std::uint16_t adr)
    {
        std::uint16_t lowByte = read(regPC);
        std::uint16_t highByte = read(regPC + 1);
        regPC = lowByte | (highByte << 8);
    }

    void CPU::opSEI(std::uint16_t adr)
    {
        regStatus = regStatus | FLAG_I;
    }
    void CPU::opCLI(std::uint16_t adr)
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

    std::uint16_t CPU::getAddressImplied()
    {
        return 0; // Simply means the instruction doesn't need an operand
    }

    void CPU::run()
    {
        while(!illegalOpcode)
            step();
    }

}