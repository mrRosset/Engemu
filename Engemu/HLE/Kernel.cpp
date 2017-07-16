#include <string>
#include <iostream>

#include "Kernel.h"
#include "E32std.h"
#include "../CPU/CPU.h"
#include "../CPU/Decoder/IR.h"

typedef u32 vir_add;

void Kernel::Executive_Call(u32 number, CPU& cpu, Gui* gui) {

	vir_add rheap = 0;

	switch (number) {	
	
	case 0x6C:
		if (!rheap) {
			rheap = cpu.mem.allocateRam(sizeof(RHeap));
		}

		//save all registers
		u32 saved_gprs[16];
		for (int i = 0; i < 16; i++) {
			saved_gprs[i] = cpu.gprs[i];
		}

		//Call RHeap::RHeap(TInt aMaxLength)
		cpu.gprs[0] = rheap;
		cpu.gprs[1] = 1052672;

		cpu.gprs[Regs::PC] =  0x503B0DAC; //TODO: not hardcode this.
		cpu.gprs[Regs::LR] = 0;

		while (cpu.gprs[Regs::PC] != 0) {
			//gui->render();
			cpu.Step();
		}

		//restore all registers
		for (int i = 0; i < 16; i++) {
			cpu.gprs[i] = saved_gprs[i];
		}

		break;
	default:
		throw std::string("non-implemented executive call");
	}

}
