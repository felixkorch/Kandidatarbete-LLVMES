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
        , internalMemory(0x800)
    {
    }

    void CPU::setExternalMemory(std::vector<std::uint8_t> mem)
    {
        externalMemory = std::move(mem);
        std::uint16_t lowByte = readMemory(ResetVector);
        std::uint16_t highByte = readMemory(ResetVector + 1);
        regPC = lowByte | (highByte << 8);
    }

    std::uint8_t CPU::readMemory(std::uint16_t address)
    {
        if(address <= 0x1FFF) {
            // %0x800 is modulus. in the scope of 0x0000 - 0x1fff is one RAM with 3 mirroring
            return internalMemory[address % 0x800];
        }
        if(address <= 0x3FFF) {
            // TODO: PPU
        }
        if(address <= 0x4017) {
            // TODO: APU / IO
        }
        if(address <= 0x401F) {
            // DISABLED
        }
        return externalMemory[address];
    }

    std::uint16_t CPU::getAddressImmediate()
    {
        return regPC++;
    }

    std::uint16_t CPU::getAddress(AddressingMode mode)
    {
        switch(mode) {
            case AddressingMode::Immediate: return getAddressImmediate();
            //case AddressingMode::Absolute: return getAddressAbsolute();
            default: return getAddressImmediate();
        }
    }

	void CPU::Execute()
    {
        switch(readMemory(regPC++)) {
            //case 0x00: return opBRK();
            //case 0x10: return opBPL();
            //case 0x20: return opJSR();
            //case 0x30: return opBMI();
            //case 0x40: return opRTI();
            //case 0x50: return opBVC();
            //case 0x60: return opRTS();
            //case 0x70: return opBVS();
            //case 0x90: return opBCC();
            case 0xA0: return opLDY(AddressingMode::Immediate);
            //case 0xB0: return opBCC();
            //case 0xC0: return opCPY(AddressingMode::Immediate);
            case 0xD0: return opBRA(regStatus.Z == 0); // BNE
												  //case 0xE0: return opCPX(AddressingMode::Immediate);
												  //case 0x01: return opORA();
												  //case 0x11: return opORA();
												  //case 0x21: return opAND();
			//case 0x31: return opAND();
			//case 0x41: return opEOR();
			//case 0x51: return opEOR();
			//case 0x61: return opADC();
			//case 0x71: return opADC();
			//case 0x81: return opSTA();
			//case 0x91: return opSTA();
			//case 0xA1: return opLDA();
			//case 0xB1: return opLDA();
			//case 0xC1: return opCMP();
			//case 0xD1: return opCMP();
			//case 0xE1: return opSBC();
			//case 0xF1: return opSBC();
			case 0xA2: return opLDX(AddressingMode::Immediate);
			case 0xE8: return opINX();
			case 0x88: return opDEY();
            case 0x69: return opADC(AddressingMode::Immediate);
            case 0x78: return opSetflag(Flag_I);
            case 0xA9: return opLDA(AddressingMode::Immediate);
            case 0xAD: return opLDA(AddressingMode::Absolute); 
            case 0xF0: return opBRA(regStatus.Z == 1); // BEQ
            case 0xEA: return opNOP();
			default:
				return illegalOP();
		}
    }

    void CPU::illegalOP()
    {
        std::cout << "Illegal OP!" << std::endl;
        std::cin.get(); // Debug measure to pause execution in terminal
    }

    void CPU::opADC(AddressingMode mode)
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
        std::cout << "INX" << std::endl;
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
        std::cout << "DEY, Flag Z: " << regStatus.Z << std::endl;
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
        std::cout << "NOP" << std::endl;

	}

    void CPU::opLDY(AddressingMode mode)
    {
        // Load index Y with memory
        std::uint16_t address = getAddress(mode);
        std::uint8_t operand = readMemory(address);
        regY = operand;
        regStatus.Z = operand == 0;
        regStatus.N = operand & 0x80;
	}

    void CPU::opLDA(AddressingMode mode)
    {
        // Load Accumulator
        std::uint16_t address = getAddress(mode);
        std::uint8_t operand = readMemory(address);
        regA = operand;
        regStatus.Z = operand == 0;
        regStatus.N = operand & 0x80;
	}

    void CPU::opLDX(AddressingMode mode)
    {
        // Load Accumulator
        std::uint16_t address = getAddress(mode);
        std::uint8_t operand = readMemory(address);
        regX = operand;
        regStatus.Z = operand == 0;
        regStatus.N = operand & 0x80;
	}

    void CPU::opJMPIndirect()
    {

    }

    void CPU::opBRA(bool condition)
    {
        std::uint16_t address = getAddressImmediate();
        std::int8_t operand = readMemory(address); // Can be negative
        if(condition) {
            std::cout << "Branch" << std::endl;
            regPC += operand;
        }
    }

    void CPU::opJMPAbsolute()
    {
        std::uint16_t lowByte = readMemory(regPC);
        std::uint16_t highByte = readMemory(regPC + 1);
        regPC = lowByte | (highByte << 8);
    }

    void CPU::opSetflag(unsigned int flag)
    {
        regStatus = regStatus | flag;
    }
    void CPU::opClearflag(unsigned int flag)
    {
		regStatus = regStatus & ~flag;
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
}