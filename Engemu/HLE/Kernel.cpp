#include <string>
#include <iostream>
#include <chrono>
#include <thread>

#include "Kernel.h"
#include "E32std.h"
#include "../CPU/CPU.h"
#include "../CPU/Decoder/IR.h"

namespace Kernel {
	vir_add RHeap_ptr = 0;
}

void Kernel::Executive_Call(u32 number, CPU& cpu, Gui* gui) {

	switch (number) {	

	case 0x6C: User_Heap(cpu, gui); break;
	case 0x8D: User_LockedInc(cpu); break;
	case 0x8E: User_LockedDec(cpu); break;
	case 0x2A: RSemaphore_Wait(cpu); break;

	default:
		throw std::string("non-implemented executive call ") + std::to_string(number);
	}

}

void Kernel::User_Heap(CPU& cpu, Gui* gui) {
	if (!RHeap_ptr) {
		RHeap_ptr = cpu.mem.allocateRam(sizeof(RHeap));
	}

	//save all registers
	u32 saved_gprs[16];
	for (int i = 0; i < 16; i++) {
		saved_gprs[i] = cpu.gprs[i];
	}

	//Call RHeap::RHeap(TInt aMaxLength)
	cpu.gprs[0] = RHeap_ptr;
	cpu.gprs[1] = 1052672;

	cpu.gprs[Regs::PC] = 0x503B0DAC; //TODO: not hardcode this.
	cpu.gprs[Regs::LR] = 0;

	while (cpu.gprs[Regs::PC] != 0) {
		cpu.Step();
		gui->render();
		std::this_thread::sleep_for(std::chrono::milliseconds(40));
	}

	//restore all registers except return value in r0
	for (int i = 1; i < 16; i++) {
		cpu.gprs[i] = saved_gprs[i];
	}

	gui->render();
}

void Kernel::User_LockedDec(CPU& cpu) {
	//TODO: Change if multithreading is implemented
	u32 value = cpu.mem.read32(cpu.gprs[0]);
	cpu.mem.write32(cpu.gprs[0], value - 1);
	cpu.gprs[0] = value;
}

void Kernel::User_LockedInc(CPU& cpu) {
	//TODO: Change if multithreading is implemented
	u32 value = cpu.mem.read32(cpu.gprs[0]);
	cpu.mem.write32(cpu.gprs[0], value + 1);
	cpu.gprs[0] = value;
}

void Kernel::RSemaphore_Wait(CPU& cpu) {
	//TODO: emulate properely Semaphore
	/*
	Problem is that for semaphore inside the rom, I don't know how, when, where
	thy are created and initialized.
	*/
}