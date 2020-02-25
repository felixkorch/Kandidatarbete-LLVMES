#pragma once
#include <string>
#include <vector>

namespace llvmes {

class ROM {
   public:
    using const_iterator = std::vector<char>::const_iterator;

    /// Initialize by reading a file.
    ROM(const std::string& path);

    /// Initialize by copying from a source.
    ROM(char* source, std::size_t length);

    /// Begin iterator of the PRGROM.
    ///
    /// The PRGROM begins after the header at byte 16.
    const_iterator BeginPRGROM() const;

    /// End iterator of the PRGROM.
    ///
    /// The PRGROM consists of blocks of 16kB each. The amount of blocks needed
    /// is located in the
    // header at byte 4.
    const_iterator EndPRGROM() const;

    /// Begin iterator of the CHRROM.
    ///
    /// The CHRROM begins where the PRGROM ends.
    const_iterator BeginCHRROM() const;

    /// End iterator of the CHRROM.
    ///
    /// The CHROM consists of blocks of 8kB each. The amount of blocks needed is
    /// located in the
    // header at byte 5.
    const_iterator EndCHRROM() const;

    bool Empty() const;
    const std::string MapperName() const;
    int MapperCode() const;

    friend std::ostream& operator<<(std::ostream& stream, const ROM& rom);

   private:
    std::vector<char> m_data;
};

}  // namespace llvmes
