#include "llvmes/NES/CPU.h"
#include <iostream>
#include <bitset>

namespace llvmes {

    CPU::CPU(BusRead read, BusWrite write)
        : regX(0)
        , regY(0)
        , regA(0)
        , regSP(0xFD)
        , regPC(0)
        , regStatus(0x34)
        , read(read)
        , write(write)
    {}

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

	void CPU::execute()
    {
        switch(read(regPC++)) {
            //case 0x00: return opBRK();
            //case 0x10: return opBPL();
            //case 0x20: return opJSR();
            //case 0x30: return opBMI();
            //case 0x40: return opRTI();
            //case 0x50: return opBVC();
            //case 0x60: return opRTS();
            //case 0x70: return opBVS();
            //case 0x90: return opBCC();
            case 0xA0: return opLDY(&CPU::getAddressImmediate);
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
			case 0xA2: return opLDX(&CPU::getAddressImmediate);
			case 0xE8: return opINX();
			case 0x88: return opDEY();
            case 0x69: return opADC(&CPU::getAddressImmediate);
            case 0x78: return opSetFlag(FLAG_I);
            case 0xA9: return opLDA(&CPU::getAddressImmediate);
            case 0xAD: return opLDA(&CPU::getAddressAbsolute);
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

    void CPU::opADC(AddressMode getAddress)
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

    void CPU::opLDY(AddressMode getAddress)
    {
        // Load index Y with memory
        std::uint16_t address = (this->*getAddress)();
        std::uint8_t operand = read(address);
        regY = operand;
        regStatus.Z = operand == 0;
        regStatus.N = operand & 0x80;
	}

    void CPU::opLDA(AddressMode getAddress)
    {
        // Load Accumulator
        std::uint16_t address = (this->*getAddress)();
        std::uint8_t operand = read(address);
        regA = operand;
        regStatus.Z = operand == 0;
        regStatus.N = operand & 0x80;
	}

    void CPU::opLDX(AddressMode getAddress)
    {
        // Load Accumulator
        std::uint16_t address = (this->*getAddress)();
        std::uint8_t operand = read(address);
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
        std::int8_t operand = read(address); // Can be negative
        if(condition) {
            std::cout << "Branch" << std::endl;
            regPC += operand;
        }
    }

    void CPU::opJMPAbsolute()
    {
        std::uint16_t lowByte = read(regPC);
        std::uint16_t highByte = read(regPC + 1);
        regPC = lowByte | (highByte << 8);
    }

    void CPU::opSetFlag(unsigned int flag)
    {
        regStatus = regStatus | flag;
    }
    void CPU::opClearFlag(unsigned int flag)
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

    void CPU::reset()
    {
        regPC = read16(RESET_VECTOR);
    }
}