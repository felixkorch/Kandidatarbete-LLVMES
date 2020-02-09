class NESInstance {
private:
    CPU cpu;
    std::vector<std::uint8_t> mem;
public:

    NESInstance()
    {
        cpu.read = std::bind(&NESInstance::readMemory, this, std::placeholders::_1);
        cpu.write = std::bind(&NESInstance::readMemory, this, std::placeholders::_2);
    }

    std::uint8_t readMemory(std::uint16_t address)
    {
        if(address <= 0x1FFF) {
            // %0x800 is modulus. in the scope of 0x0000 - 0x1fff is one RAM with 3 mirroring
            return mem[address % 0x800];
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
        return mem[address];
    }

    void writeMemory(std::uint16_t address, std::uint8_t)
    {

    }
};