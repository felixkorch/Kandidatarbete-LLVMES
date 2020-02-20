#pragma once
#include "llvmes/NES/StatusRegister.h"
#include <cstdint>
#include <vector>
#include <array>
#include <string>
#include <functional>

namespace llvmes {

    struct CPUState {
        std::uint8_t   regX;
        std::uint8_t   regY;
        std::uint8_t   regA;
        std::uint8_t   regSP;
        std::uint16_t  regPC;
        unsigned int regStatus;
    };

    class CPU {
    public:
        typedef std::function<std::uint8_t(std::uint16_t)> BusRead;
        typedef std::function<void(std::uint16_t, std::uint8_t)> BusWrite;

        CPU();
        void step();
        void run();
        void reset();
        void dump();
        void setNMI();
        void setIRQ();
        CPUState getState() { return { regX, regY, regA, regSP, regPC, regStatus }; }

        BusRead read;
        BusWrite write;

	private:

        constexpr static unsigned int FLAG_C      = (1 << 0);
        constexpr static unsigned int FLAG_Z      = (1 << 1);
        constexpr static unsigned int FLAG_I      = (1 << 2);
        constexpr static unsigned int FLAG_D      = (1 << 3);
        constexpr static unsigned int FLAG_B      = (1 << 4);
        constexpr static unsigned int FLAG_UNUSED = (1 << 5);
        constexpr static unsigned int FLAG_V      = (1 << 6);
        constexpr static unsigned int FLAG_N      = (1 << 7);

        constexpr static unsigned int NMI_VECTOR   = 0xFFFA;
        // Address at this location points to the first instruction
        constexpr static unsigned int RESET_VECTOR = 0xFFFC;
        constexpr static unsigned int IRQ_VECTOR   = 0xFFFE;

        // Pointer to a function which will execute an instruction
        typedef void(CPU::*OpFunction)();
        // Pointer to a function which will fetch an address used in an instruction
        typedef void(CPU::*AddrFunction)();

        struct Instruction {
            AddrFunction addr;
            OpFunction op;
            std::string name;
        };

    private:
        std::uint8_t   regX;
        std::uint8_t   regY;
        std::uint8_t   regA;
        std::uint8_t   regSP;
        std::uint16_t  regPC;
        StatusRegister regStatus;

        bool irq, nmi;

        // Will be set to true whenever an illegal op-code gets fetched
        bool illegalOPCode;
        // Contains the address associated with an instruction
        std::uint16_t address;
        std::vector<Instruction> instructionTable;

    private:

        /// Does two consecutively reads at a certain address
        std::uint16_t read16(std::uint16_t addr);

        void stackPush(std::uint8_t value);
        std::uint8_t stackPop();

        void invokeIRQ();
        void invokeNMI();

        void addressModeImmediate();
        void addressModeAbsolute();
        void addressModeAbsoluteX();
        void addressModeAbsoluteY();
        void addressModeZeropage();
        void addressModeZeropageX();
        void addressModeZeropageY();
        void addressModeIndirect();
        void addressModeIndirectX();
        void addressModeIndirectY();
        void addressModeImplied();
        void addressModeAccumulator();

        void opBIT();
        void opAND();
        void opEOR();
        void opORA();
        void opASL();
        void opASLAcc();
        void opLSR();
        void opLSRAcc();
        void opADC();
        void opJSR();
        void opJMP();
        void opBNE();
        void opBEQ();
        void opBMI();
        void opBCC();
        void opBCS();
        void opBPL();
        void opBVC();
        void opBVS();
		void opBRK();
		void opLDY();
		void opLDA();
		void opLDX();
        void opINX();
        void opINC();
        void opDEC();
        void opINY();
        void opDEY();
        void opDEX();
		void opNOP();
		void opSEI();
		void opCLI();
        void opCLC();
        void opCLD();
        void opCLV();
		void opPHA();
		void opPHP();
		void opPLA();
		void opPLP();
		void opROL();
		void opROR();
        void opROLAcc();
		void opRORAcc();
		void opRTI();
		void opRTS();
		void opSBC();
		void opSEC();
		void opSED();
		void opSTA();
		void opSTX();
		void opSTY();
		void opTAX();
		void opTAY();
		void opTSX();
		void opTYA();
		void opTXS();
		void opTXA();
		void opCMP();
		void opCPX();
		void opCPY();
        void illegalOP();
    };

}