#include "llvmes/nes/rom.h"
#include <iostream>

using namespace llvmes;

int main(int argc, char** argv) try
{
	if (argc == 1) {
		std::cout << "No path provided." << std::endl;
		return 1;
	}
	else if (argc > 2) {
		std::cout << "Only one argument allowed." << std::endl;
		return 1;
	}

    ROM rom(argv[1]);
    std::cout << rom << std::endl;

    return 0;

} catch(std::exception& e){
    std::cerr << e.what() << std::endl;
}
