//
// Created by Markus Pettersson on 2020-02-18.
//
#include "llvmes/NES/ProgramLoader.h"
#include "llvmes/NES/ProgramToken.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>

namespace llvmes {

    ProgramLoader::ProgramLoader(char *source, std::size_t length) {
        if (source == nullptr)
            throw "ProgramLoader: Source null";
        std::copy(source, source + length, data.begin());
    }

    ProgramLoader::ProgramLoader(const std::string &path) {
        // Open file for reading
        std::ifstream readFrom(path, std::ios::binary);

        // Check if can read from file
        if (readFrom.fail())
            throw "ProgramLoader: The file doesn't exist";

        // Read content of file
        auto content = std::vector<char>{std::istreambuf_iterator<char>(readFrom), std::istreambuf_iterator<char>()};
        // Close file
        readFrom.close();

        // Remove newlines
        content.erase(std::remove(content.begin(), content.end(), '\n'), content.end());
        // Move ownership of content to data
        data = std::move(content);
    }

    // Return (raw) content of read file
    std::vector<char> ProgramLoader::getProgram() {
        std::vector<char> data_copy;
        // Copy existing data vector
        data_copy.assign(data.begin(), data.end());
        return data_copy;
    }

    // Return content of read file which is easier to work with
    std::vector<ProgramToken *> ProgramLoader::getProgramTokens() {
        size_t programLength = data.size();
        // TODO: This is probably wrong
        std::vector<ProgramToken *> tokens;

        // TODO: This is probably wrong
        for (size_t index = 0; index < programLength; index += 2) {
            char firstToken = data[index];
            char secondToken = data[index + 1];
            ProgramToken *token = new ProgramToken(firstToken, secondToken);
            tokens.push_back(token);
        }

        return tokens;
    }

}