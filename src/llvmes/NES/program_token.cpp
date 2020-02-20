//
// Created by Markus Pettersson on 2020-02-20.
//

#include <cstdint>
#include <ios>
#include "program_token.h"

namespace llvmes {

    ProgramToken::ProgramToken(char upper, char lower) {
        this->m_token = std::string(1, upper) + lower;
    }

    // Converts the hexadecimal string representation to an integer which the emulator can interpret
    uint8_t ProgramToken::ToInt() {
        return std::stoi(this->m_token, 0, 16);
    }
}