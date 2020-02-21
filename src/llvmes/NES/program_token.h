//
// Created by Markus Pettersson on 2020-02-20.
//

/*
 * This class represents a single piece of 6502 machine code, which can either be an instruction or some data
 * */

#pragma once
#include <string>


namespace llvmes {
    class ProgramToken {

    public:
        ProgramToken(char upper, char lower);
        uint8_t ToInt();

    private:
        std::string m_token;
    };


}
