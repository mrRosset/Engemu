#include <iostream>
#include "Common.h"
#include "CPU/CPU.h"
#include "E32Image.h"
#include "Loader/E32ImageLoader.h"

int main(int argc, char* argv[])
{
	if (argc < 3) {
		std::cout << "Error missing E32Image and library folder path" << std::endl;
		return -1;
	}
	E32Image image;
	E32ImageLoader::parse(argv[1], image);
	
	CPU cpu;
	E32ImageLoader::load(image, cpu.mem, argv[2]);

    return 0;
}

