//
// Created by Markus Pettersson on 2020-02-18.
//
#include "llvmes/NES/program_loader.h"
#include "llvmes/NES/program_token.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>

namespace llvmes {

    ProgramLoader::ProgramLoader(char *source, std::size_t length) {
        if (source == nullptr)
            throw std::runtime_error("ProgramLoader: Source null");
        std::copy(source, source + length, m_data.begin());
    }

    ProgramLoader::ProgramLoader(const std::string &path) {
        // Open file for reading
        std::ifstream read_from(path, std::ios::binary);

        // Check if can read from file
        if (read_from.fail())
            throw std::runtime_error("ProgramLoader: The file doesn't exist");

        // Read content of file
        auto content = std::vector<char>{std::istreambuf_iterator<char>(read_from), std::istreambuf_iterator<char>()};
        // Close file
        read_from.close();

        // Remove newlines
        content.erase(std::remove(content.begin(), content.end(), '\n'), content.end());
        // Move ownership of content to data
        m_data = std::move(content);
    }

    // Get machine code which the emulator can interpret
    std::vector<std::uint8_t> ProgramLoader::GetProgram() {
        if (m_data.empty())
            throw std::runtime_error("ProgramLoader: The program has not been loaded or is empty!");

        auto program_tokens = this->GetProgramTokens();

        // Allocate enough memory for the program
        std::vector<std::uint8_t> program;
        program.reserve(program_tokens.size());

        for (ProgramToken *programToken : program_tokens) {
            program.push_back(programToken->ToInt());
        }

        // Cleanup program tokens before returning
        for(ProgramToken* p : program_tokens)
            delete p;

        return program;

    }

    // Return a copy of (raw) content of read file
    std::vector<char> ProgramLoader::GetFileContent() {
        std::vector<char> data_copy;

        data_copy.assign(m_data.begin(), m_data.end());
        return data_copy;
    }

    // Return content of read file as tokens, which represent hexadecimal values
    std::vector<ProgramToken *> ProgramLoader::GetProgramTokens() {
        size_t program_length = m_data.size();

        std::vector<ProgramToken *> tokens;
        tokens.reserve(program_length);

        // TODO: This assumes that there's an even number of character in the file being read.
        // There's got to be a better way!
        for (size_t index = 0; index < program_length; index += 2) {
            // E.g. 0x41 => upper_hex_token == 4; lower_hex_token == 1;
            char upper_hex_token = m_data[index];
            char lower_hex_token = m_data[index + 1];
            ProgramToken *token = new ProgramToken(upper_hex_token, lower_hex_token);
            tokens.push_back(token);
        }

        return tokens;
    }

}
