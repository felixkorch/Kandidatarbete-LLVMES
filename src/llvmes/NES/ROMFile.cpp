#include "llvmes/NES/ROMFile.h"
#include "llvmes/FileUtilities.h"

#include <string>

namespace llvmes {

	ROMFile::ROMFile(char* source, std::size_t length)
	{
        std::copy(source, source + length, data.begin());
	}

	ROMFile::ROMFile(const std::string& path)
        : data(util::readFile(path))
	{}

	ROMFile::const_iterator ROMFile::beginPRGROM() const
    {
        return std::next(data.cbegin(), 16);
    }

    ROMFile::const_iterator ROMFile::endPRGROM() const
    {
        return std::next(beginPRGROM(), data[4] * 0x4000);
    }

    ROMFile::const_iterator ROMFile::beginCHRROM() const
    {
        return endPRGROM();
    }

    ROMFile::const_iterator ROMFile::endCHRROM() const
    {
        return std::next(beginCHRROM(), data[5] * 0x2000);
    }

    bool ROMFile::empty() const
    {
        return data.size() == 0;
    }

	int ROMFile::mapperCode() const
	{
		return (data[7] & 0xF0) | (data[6] >> 4);
	}

    const std::string ROMFile::mapperName() const
    {
        switch (mapperCode()) {
        case 0: return ((endPRGROM() - beginPRGROM() > 0x4000) ? "NROM256" : "NROM128");
        case 1: return "MMC1";
        case 2: return "UxROM";
        case 4: return "MMC3";
        default: return "Unknown mapper";
        }
    }

    std::ostream& operator<<(std::ostream& stream, const ROMFile& rom)
    {
        return stream << "ROM-Layout: {\n"
            << "    MapperType: " << rom.mapperName() << " (Code: " << rom.mapperCode() << "),\n"
            << "    PRGROM Size: " << (rom.endPRGROM() - rom.beginPRGROM()) << " B,\n"
            << "    CHRROM Size: " << (rom.endCHRROM() - rom.beginCHRROM()) << " B\n"
            << "}";
    }

}