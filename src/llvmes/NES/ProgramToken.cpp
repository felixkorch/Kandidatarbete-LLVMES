//
// Created by Markus Pettersson on 2020-02-20.
//

#include <cstdint>
#include <ios>
#include "ProgramToken.h"

namespace llvmes {

    ProgramToken::ProgramToken(char upper, char lower) {
        this->token = std::string(1, upper) + lower;
    }

    // Converts the hexadecimal string representation to an integer which the emulator can interpret
    uint8_t ProgramToken::toInt() {
        return std::stoi(this->token, 0, 16);
    }
}