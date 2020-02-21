//
// Created by Markus Pettersson on 2020-02-18.
//

#pragma once
// #include <cstddef>
// #include <string>
#include <vector>
#include "program_token.h"

namespace llvmes {
    class ProgramLoader {

    public:
        ProgramLoader(char *source, std::size_t length);

        ProgramLoader(const std::string &path);

        std::vector<std::uint8_t> GetProgram();

    private:
        std::vector<ProgramToken*> GetProgramTokens();
        std::vector<char> GetFileContent();
        std::vector<char> m_data;
    };
}
