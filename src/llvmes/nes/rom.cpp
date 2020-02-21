#include "llvmes/nes/rom.h"
#include <fstream>
namespace llvmes {

    ROM::ROM(char* source, std::size_t length)
	{
        if(source == nullptr)
            throw std::runtime_error("ROM: Source NULL");
        std::copy(source, source + length, m_data.begin());
	}

    ROM::ROM(const std::string& path)
	{
        std::ifstream in{ path, std::ios::binary };
        if (in.fail())
            throw std::runtime_error("ROM: The file doesn't exist");
        m_data = std::vector<char>{ std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>() };
	}

    ROM::const_iterator ROM::BeginPRGROM() const
    {
        return std::next(m_data.cbegin(), 16);
    }

    ROM::const_iterator ROM::EndPRGROM() const
    {
        return std::next(BeginPRGROM(), m_data[4] * 0x4000);
    }

    ROM::const_iterator ROM::BeginCHRROM() const
    {
        return EndPRGROM();
    }

    ROM::const_iterator ROM::EndCHRROM() const
    {
        return std::next(BeginCHRROM(), m_data[5] * 0x2000);
    }

    bool ROM::Empty() const
    {
        return m_data.size() == 0;
    }

	int ROM::MapperCode() const
	{
		return (m_data[7] & 0xF0) | (m_data[6] >> 4);
	}

    const std::string ROM::MapperName() const
    {
        switch (MapperCode()) {
        case 0: return ((EndPRGROM() - BeginPRGROM() > 0x4000) ? "NROM256" : "NROM128");
        case 1: return "MMC1";
        case 2: return "UxROM";
        case 4: return "MMC3";
        default: return "Unknown mapper";
        }
    }

    std::ostream& operator<<(std::ostream& stream, const ROM& rom)
    {
        return stream << "ROM-Layout: {\n"
            << "    MapperType: " << rom.MapperName() << " (Code: " << rom.MapperCode() << "),\n"
            << "    PRGROM Size: " << (rom.EndPRGROM() - rom.BeginPRGROM()) << " B,\n"
            << "    CHRROM Size: " << (rom.EndCHRROM() - rom.BeginCHRROM()) << " B\n"
            << "}";
    }

}
