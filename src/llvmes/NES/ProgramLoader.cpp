//
// Created by Markus Pettersson on 2020-02-18.
//
#include "llvmes/NES/ProgramLoader.h"
#include <fstream>
#include <vector>

namespace llvmes {

    ProgramLoader::ProgramLoader(char* source, std::size_t length)
    {
        if(source == nullptr)
            throw "ProgramLoader: Source null";
        std::copy(source, source + length, data.begin());
    }

    ProgramLoader::ProgramLoader(const std::string& path)
    {
        std::ifstream in{ path, std::ios::binary };
        if (in.fail())
            throw "ProgramLoader: The file doesn't exist";
        auto temp = std::vector<char>{ std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>() };
        data = std::move(temp);
    }



}