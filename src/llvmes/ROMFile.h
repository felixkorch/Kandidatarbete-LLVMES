// ---------------------------------------------------------------------* C++ *-
// ROMFile.h
//
// A class to represent a ROM File
//
// -----------------------------------------------------------------------------

#pragma once
#include "llvmes/FileUtilities.h"
#include <vector>
#include <string>

namespace llvmes {

    class ROMFile {
        std::vector<char> data;
    public:
        using const_iterator = decltype(data)::const_iterator;

        /// Initialize by reading a file.
        ROMFile(const std::string& path)
            : data(util::ReadFile(path))
        {}

        /// Initialize by copying from a source.
        ROMFile(std::uint8_t* source, std::size_t length)
            : data(length)
        {
            std::copy(source, source + length, data.begin());
        }

        unsigned int mapperCode() const
        {
            return (data[7] & 0xF0) | (data[6] >> 4);
        }

        /// Begin iterator of the PRGROM.
        ///
        /// The PRGROM begins after the header at byte 16.
        const_iterator beginPRGROM() const
        {
            return std::next(data.cbegin(), 16);
        }

        /// End iterator of the PRGROM.
        ///
        /// The PRGROM consists of blocks of 16kB each. The amount of blocks needed is located in the
        // header at byte 4.
        const_iterator endPRGROM() const
        {
            return std::next(beginPRGROM(), data[4] * 0x4000);
        }

        /// Begin iterator of the CHRROM.
        ///
        /// The CHRROM begins where the PRGROM ends.
        const_iterator beginCHRROM() const
        {
            return endPRGROM();
        }

        /// End iterator of the CHRROM.
        ///
        /// The CHROM consists of blocks of 8kB each. The amount of blocks needed is located in the
        // header at byte 5.
        const_iterator endCHRROM() const
        {
            return std::next(beginCHRROM(), data[5] * 0x2000);
        }

        bool isInitialized()
        {
            return data.size() > 0;
        }

    };

    struct MapperType {
        const ROMFile& ROMRef;
        MapperType(const ROMFile& ROMRef)
            : ROMRef(ROMRef)
        {}
    };

    inline std::ostream& operator<<(std::ostream& stream, const MapperType& mapperType)
    {
        const ROMFile& rom = mapperType.ROMRef;
        switch (rom.mapperCode()) {
        case 0: return stream << ((rom.endPRGROM() - rom.beginPRGROM() > 0x4000) ? "NROM 256" : "NROM128"); break;
        case 1: return stream << "MMC1"; break;
        case 2: return stream << "UxROM"; break;
        case 4: return stream << "MMC3"; break;
        default: return stream << "Unknown mapper"; break;
        }
    }

    inline std::ostream& operator<<(std::ostream& stream, const ROMFile& rom)
    {
        return stream << "ROM-Layout: {\n"
            << "    MapperType: " << MapperType(rom) << " ( Code: " << rom.mapperCode() << " ),\n"
            << "    PRGROM Size: " << (rom.endPRGROM() - rom.beginPRGROM()) << " B,\n"
            << "    CHRROM Size: " << (rom.endCHRROM() - rom.beginCHRROM()) << " B\n"
            << "}";
    }

}