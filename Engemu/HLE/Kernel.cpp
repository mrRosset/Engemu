#include <string>
#include <iostream>
#include <chrono>
#include <thread>

#include "Kernel.h"
#include "E32std.h"
#include "../Emulator.h"
#include "../CPU/Tharm/CPU.h"
#include "../CPU/Tharm/Decoder/IR.h"
#include "../CPU/Unicorn/CPUnicorn.h"

namespace Kernel {
	vir_add RHeap_ptr = 0;
	vir_add TrapHandler_ptr = 0;
}

void Kernel::Executive_Call(u32 number, Emulator& emu, GuiMain* gui) {
	auto& cpu = emu.getCPU();

	switch (number) {
	case 0x4D: User__WaitForAnyRequest(cpu); break;
	case 0x6C: User_Heap(emu, gui); break;
	case 0x8D: User_LockedInc(cpu); break;
	case 0x8E: User_LockedDec(cpu); break;
	case 0x2A: RSemaphore_Wait(cpu); break;
	case 0x53: TChar_GetCategory(cpu); break;
	case 0x52: TChar_GetUpperCase(cpu);  break;
	case 0x82: User_SetTrapHandler(cpu); break;

	case 0x8000C0: RProcess_CommandLineLength(cpu); break;
	case 0xC00076: UserSvr__InitRegisterCallback(cpu, gui); break;

	default:

		throw std::string("non-implemented executive call ") + std::to_string(number);
	}

}

void Kernel::User_Heap(Emulator& emu, GuiMain* gui) {
	auto& cpu = emu.getCPU();

	if (RHeap_ptr) {
		cpu.SetReg(0, RHeap_ptr);
		return;
	}
	
	RHeap_ptr = cpu.mem.allocateRam(sizeof(RHeap));

	CPU ker_cpu(cpu.mem);

	ker_cpu.gprs[0] = RHeap_ptr;
	ker_cpu.gprs[1] = 1052672; //Found in a test on the hardware

	ker_cpu.gprs[Regs::PC] = 0x503B0DAC; //TODO: not hardcode this.
	ker_cpu.gprs[Regs::LR] = 0;
	ker_cpu.gprs[Regs::SP] = cpu.GetReg(Regs::SP);
	
	ker_cpu.call_stack.push_back("0x503B0DAC");

	emu.setCPU(ker_cpu);

	while (ker_cpu.gprs.RealPC() != 0) {
		emu.Step();
		gui->render();
		std::this_thread::sleep_for(std::chrono::milliseconds(40));
	}

	emu.setCPU(cpu);

	//get the return value
	cpu.SetReg(0, ker_cpu.gprs[0]);
}

void Kernel::User_LockedDec(CPU_Interface& cpu) {
	//TODO: Change if multithreading is implemented
	u32 value = cpu.mem.read32(cpu.GetReg(0));
	cpu.mem.write32(cpu.GetReg(0), value - 1);
	cpu.SetReg(0, value);
}

void Kernel::User_LockedInc(CPU_Interface& cpu) {
	//TODO: Change if multithreading is implemented
	u32 value = cpu.mem.read32(cpu.GetReg(0));
	cpu.mem.write32(cpu.GetReg(0), value + 1);
	cpu.SetReg(0, value);
}

void Kernel::RSemaphore_Wait(CPU_Interface& cpu) {
	//TODO: emulate properely Semaphore
	/*
	Problem is that for semaphore inside the rom, I don't know how, when, where
	thy are created and initialized.
	*/
}

void Kernel::RProcess_CommandLineLength(CPU_Interface& cpu) {
	cpu.SetReg(0, 43);
}

void Kernel::User_SetTrapHandler(CPU_Interface& cpu) {
	vir_add oldHandler = TrapHandler_ptr;
	TrapHandler_ptr = cpu.GetReg(0);
	cpu.SetReg(0, oldHandler);
}

void Kernel::UserSvr__InitRegisterCallback(CPU_Interface& cpu, GuiMain* gui) {
	//TODO understand how this work. If this is even correct.
	throw std::string("test");
}

void Kernel::User__WaitForAnyRequest(CPU_Interface& cpu) {
	//TODO: Should wait until a request complete and execution can resume.
	throw std::string("lol");
}

void Kernel::CallSWIHandler(Emulator& emu, GuiMain* gui) {
	auto& cpu = emu.getCPU();

	CPUnicorn ker_cpu((GageMemory&)cpu.mem);
	ker_cpu.SetPC(0x50321490);
	ker_cpu.SetReg(Regs::LR, cpu.GetPC());
	ker_cpu.SetReg(Regs::SP, cpu.GetReg(Regs::SP));

	emu.state = CPUState::Stopped;

	emu.setCPU(ker_cpu);

	while (ker_cpu.GetPC() != 0) {
		emu.Step();
		gui->render();
	}

	emu.setCPU(cpu);
}
