#include "llvmes/nes/rom.h"
#include <fstream>
namespace llvmes {

    ROM::ROM(char* source, std::size_t length)
	{
        if(source == nullptr)
            throw std::runtime_error("ROM: Source NULL");
        std::copy(source, source + length, data.begin());
	}

    ROM::ROM(const std::string& path)
	{
        std::ifstream in{ path, std::ios::binary };
        if (in.fail())
            throw std::runtime_error("ROM: The file doesn't exist");
        data = std::vector<char>{ std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>() };
	}

    ROM::const_iterator ROM::beginPRGROM() const
    {
        return std::next(data.cbegin(), 16);
    }

    ROM::const_iterator ROM::endPRGROM() const
    {
        return std::next(beginPRGROM(), data[4] * 0x4000);
    }

    ROM::const_iterator ROM::beginCHRROM() const
    {
        return endPRGROM();
    }

    ROM::const_iterator ROM::endCHRROM() const
    {
        return std::next(beginCHRROM(), data[5] * 0x2000);
    }

    bool ROM::empty() const
    {
        return data.size() == 0;
    }

	int ROM::mapperCode() const
	{
		return (data[7] & 0xF0) | (data[6] >> 4);
	}

    const std::string ROM::mapperName() const
    {
        switch (mapperCode()) {
        case 0: return ((endPRGROM() - beginPRGROM() > 0x4000) ? "NROM256" : "NROM128");
        case 1: return "MMC1";
        case 2: return "UxROM";
        case 4: return "MMC3";
        default: return "Unknown mapper";
        }
    }

    std::ostream& operator<<(std::ostream& stream, const ROM& rom)
    {
        return stream << "ROM-Layout: {\n"
            << "    MapperType: " << rom.mapperName() << " (Code: " << rom.mapperCode() << "),\n"
            << "    PRGROM Size: " << (rom.endPRGROM() - rom.beginPRGROM()) << " B,\n"
            << "    CHRROM Size: " << (rom.endCHRROM() - rom.beginCHRROM()) << " B\n"
            << "}";
    }

}
