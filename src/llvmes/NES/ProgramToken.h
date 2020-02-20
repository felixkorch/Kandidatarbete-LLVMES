//
// Created by Markus Pettersson on 2020-02-20.
//

/*
 * This class represents a single Instruction of 6502 machine code
 * */

#ifndef LLVMES_PROGRAMTOKEN_H
#define LLVMES_PROGRAMTOKEN_H


namespace llvmes {
    class ProgramToken {

    public:
        ProgramToken(char upper, char lower);
        uint8_t toInt();

    private:
        std::string token;
    };


}

#endif //LLVMES_PROGRAMTOKEN_H
