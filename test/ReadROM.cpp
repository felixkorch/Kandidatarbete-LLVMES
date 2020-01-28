#include "llvmes/ROMFile.h"
#include <iostream>

using namespace llvmes;

int main(int argc, char** argv)
{
	if (argc == 1) {
		std::cout << "No path provided." << std::endl;
		return 1;
	}
	else if (argc > 2) {
		std::cout << "Only one argument allowed." << std::endl;
		return 1;
	}

	ROMFile rom(argv[1]);
	if(rom.isInitialized())
		std::cout << rom << std::endl;
	return 0;
}