#include "llvmes/NES/StatusRegister.h"
#include <cstdint>
#include <vector>

namespace llvmes {

    class CPU {
    public:
        CPU();
        void setExternalMemory(std::vector<std::uint8_t> mem);
        void Execute();
        const std::vector<std::uint8_t>& getMemory() const { return internalMemory; }
        void dump();

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
        std::vector<std::uint8_t> internalMemory, externalMemory;

        /// This type is a function pointer with [return type: 16bit unsigned, no arguments]
        /// It will return an address
        typedef std::uint16_t (CPU::*AddressMode)();

        /// Get the address using different addressing modes.
        std::uint16_t getAddressImmediate();
        std::uint16_t getAddressAbsolute();

    private:
        /// Read/Write Memory
        std::uint8_t readMemory(std::uint16_t adr);
        std::uint16_t read16(std::uint16_t adr);
        std::uint8_t writeMemory(std::uint16_t adr);

        /// Declarations of the various instructions.
        void opADC(AddressMode getAddress);
        void opJMPIndirect();
        void opJMPAbsolute();
        void opBRA(bool condition);
		void opBRK();
		void opLDY(AddressMode getAddress);
		void opLDA(AddressMode getAddress);
		void opLDX(AddressMode getAddress);
        void opINX();
        void opINY();
        void opDEY();
        void opDEX();
		void opNOP();
		void opSetFlag(unsigned int flag);
		void opClearFlag(unsigned int flag);

        void illegalOP();
    };

}