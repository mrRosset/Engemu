#include "Emulator.h"

#include <iostream>


std::string extract_appname(const std::string& filepath) {
	auto pos = filepath.rfind("\\");
	if (pos == std::string::npos)
		pos = -1;
	return std::string(filepath.begin() + pos + 1, filepath.end());
}


Emulator::Emulator(Memory_Interface & mem_, CPU_Interface & cpu_, std::string & app_path, std::string & lib_folder_path, std::string & rom_path):
	mem(mem_), cpu(&cpu_) {

	E32ImageLoader::parse(app_path, image);

	mem.loadRom(rom_path);
	//E32ImageLoader::load(image, extract_appname(app_path), mem, lib_folder_path);


	//cpu->SetPC(image.header->code_base_address + image.header->entry_point_offset); // 0x50392D54 <- entry of Euser.dll;
	//cpu->SetPC(image.header->code_base_address + image.code_section.export_directory[0]);
	//cpu->SetPC(0x5063D444); //Main of AppRun
	cpu->SetPC(0x0);

	//PSR& cpsr = cpu->GetCPSR();
	//cpsr.flag_T = true;
	//cpu->SetCPSR(cpsr);

	//TODO: find the correct place where the SP is initialized
	//cpu->gprs[Regs::SP] = 0x7FFF'FFFF; //start of the ram section
	cpu->SetReg(Regs::SP, 0x7FFFFFFC); //start of the ram section aligned with last 2 bit 0

	state = CPUState::Stopped;
}

void Emulator::Step() {
	try {
		switch (state) {

		case CPUState::Step:
			cpu->Step();
			state = CPUState::Stopped;
			break;

		case CPUState::Running:
			cpu->Step();
			break;
		}
	}
	catch (std::string& error_message) {
		std::cout << "Uncaught exception:\n" << error_message << std::endl;
		state = CPUState::Stopped;
	}
	catch (const char* error_message) {
		std::cout << "Uncaught exception:\n" << error_message << std::endl;
		state = CPUState::Stopped;
	}
}

CPU_Interface& Emulator::getCPU() {
	return *cpu;
}

void Emulator::setCPU(CPU_Interface& cpu_) {
	cpu = &cpu_;
}
