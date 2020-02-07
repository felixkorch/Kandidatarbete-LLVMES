#pragma once
#include <vector>
#include <fstream>
#include <iostream>

namespace llvmes {
	namespace util {

		inline std::vector<char> readFile(const std::string& path)
		{
			std::ifstream in{ path, std::ios::binary };
			if (in.fail()) {
				std::cout << "There was a problem reading the requested file." << std::endl;
				return std::vector<char>{};
			}
			return std::vector<char>{ std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>() };
		}

		inline void writeFile(const std::string& path, const std::vector<char>& source)
		{
			std::ofstream output{ path, std::ios::binary };
			output.write(source.data(), source.size());
		}

	}
}