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
		constexpr static unsigned int Flag_C      = (1 << 0);
		constexpr static unsigned int Flag_Z      = (1 << 1);
        constexpr static unsigned int Flag_I      = (1 << 2);
        constexpr static unsigned int Flag_D      = (1 << 3); // Disabled on the NES (decimal mode).
        constexpr static unsigned int Flag_B      = (1 << 4); // Bits 4 and 5 are used to indicate whether a
        constexpr static unsigned int Flag_Unused = (1 << 5); // Software or hardware interrupt occured
        constexpr static unsigned int Flag_V      = (1 << 6);
        constexpr static unsigned int Flag_N      = (1 << 7);

        constexpr static unsigned int ResetVector = 0xFFFC; // Address at this location will be executed first
        constexpr static unsigned int NMIVector   = 0xFFFA;
        constexpr static unsigned int IRQVector   = 0xFFFE;

        std::uint8_t   regX;
        std::uint8_t   regY;
        std::uint8_t   regA;
        std::uint8_t   regSP;
        std::uint16_t  regPC;
        StatusRegister regStatus;
        std::vector<std::uint8_t> internalMemory, externalMemory;

        enum class AddressingMode {
            Immediate,
            Absolute
        };

        /// Get the address using different addressing modes.
        std::uint16_t getAddressImmediate();

        /// Calls the appropriate Addressing function depending on the enum.
        std::uint16_t getAddress(AddressingMode mode);

    private:
        /// Read/Write Memory
        std::uint8_t readMemory(std::uint16_t adr);
        std::uint8_t writeMemory(std::uint16_t adr);

        /// Declarations of the various instructions.
        void opADC(AddressingMode mode);
        void opJMPIndirect();
        void opJMPAbsolute();
        void opBRA(bool condition);
		void opBRK();
		void opLDY(AddressingMode mode);
		void opLDA(AddressingMode mode);
		void opLDX(AddressingMode mode);
        void opINX();
        void opINY();
        void opDEY();
        void opDEX();
		void opNOP();
		void opSetflag(unsigned int flag);
		void opClearflag(unsigned int flag);

        void illegalOP();
    };

}