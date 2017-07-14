#include <string>
#include <iostream>

#include "Kernel.h"
#include "E32std.h"
#include "../CPU/CPU.h"
#include "../CPU/Decoder/IR.h"

void Kernel::Executive_Call(CPU& cpu, u32 number) {

	switch (number) {	
	case 0x6C:
		std::cout << "hello";
		break;
	default:
		throw std::string("non-implemented executive call");
	}

}
