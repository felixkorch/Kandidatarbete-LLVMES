//
// Created by Markus Pettersson on 2020-02-18.
//

#ifndef LLVMES_PROGRAMLOADER_H
#define LLVMES_PROGRAMLOADER_H

#include <cstddef>
#include <string>
#include <vector>
#include "ProgramToken.h"

namespace llvmes {
    class ProgramLoader {

    public:
        ProgramLoader(char *source, std::size_t length);

        ProgramLoader(const std::string &path);

        ~ProgramLoader();

        std::vector<uint8_t> getProgram();

    private:
        std::vector<ProgramToken*> getProgramTokens();
        std::vector<char> getFileContent();
        std::vector<char> data;
    };
}

#endif //LLVMES_PROGRAMLOADER_H
