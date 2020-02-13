#pragma once
#include "llvmes/NES/StatusRegister.h"
#include <cstdint>
#include <vector>
#include <array>

namespace llvmes {

    class CPU {
    public:
        typedef std::function<std::uint8_t(std::uint16_t)> BusRead;
        typedef std::function<void(std::uint16_t, std::uint8_t)> BusWrite;

        // Publicly exposed functions
        CPU();
        void step();
        void run();
        void reset();
        void dump();

        BusRead read;
        BusWrite write;

	private:

		constexpr static unsigned int FLAG_C      = (1 << 0);
		constexpr static unsigned int FLAG_Z      = (1 << 1);
        constexpr static unsigned int FLAG_I      = (1 << 2);
        constexpr static unsigned int FLAG_D      = (1 << 3); // Disabled on the NES (decimal mode).
        constexpr static unsigned int FLAG_B      = (1 << 4); // Bits 4 and 5 are used to indicate whether a
        constexpr static unsigned int FLAG_UNUSED = (1 << 5); // Software or hardware interrupt occurred
        constexpr static unsigned int FLAG_V      = (1 << 6);
        constexpr static unsigned int FLAG_N      = (1 << 7);

        constexpr static unsigned int RESET_VECTOR = 0xFFFC; // Address at this location points to the first instruction
        constexpr static unsigned int NMI_VECTOR   = 0xFFFA;
        constexpr static unsigned int IRQ_VECTOR   = 0xFFFE;

        std::uint8_t   regX;
        std::uint8_t   regY;
        std::uint8_t   regA;
        std::uint8_t   regSP;
        std::uint16_t  regPC;
        StatusRegister regStatus;

        typedef void(CPU::*OpFunction)(std::uint16_t);
        typedef std::uint16_t(CPU::*AddrFunction)();

        struct Instruction {
            AddrFunction addr;
            OpFunction op;
        };

        std::vector<Instruction> instructionTable;

        bool illegalOpcode; // Will be set to true whenever an illegal op-code gets fetched

        /// Get the address using different addressing modes.
        std::uint16_t getAddressImmediate();
        std::uint16_t getAddressAbsolute();
        std::uint16_t getAddressImplied();

    private:
        /// Does two consecutively reads at a certain address
        std::uint16_t read16(std::uint16_t addr);

        /// Declarations of the various instructions.
        void opADC(std::uint16_t addr);
        void opJMPIndirect(std::uint16_t addr);
        void opJMPAbsolute(std::uint16_t addr);
        void opBNE(std::uint16_t addr);
        void opBEQ(std::uint16_t addr);
		void opBRK(std::uint16_t addr);
		void opLDY(std::uint16_t addr);
		void opLDA(std::uint16_t addr);
		void opLDX(std::uint16_t addr);
        void opINX(std::uint16_t addr);
        void opINY(std::uint16_t addr);
        void opDEY(std::uint16_t addr);
        void opDEX(std::uint16_t addr);
		void opNOP(std::uint16_t addr);
		void opSEI(std::uint16_t addr);
		void opCLI(std::uint16_t addr);
        void illegalOP(std::uint16_t addr);
    };

}