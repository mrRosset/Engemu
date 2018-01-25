#pragma once

#include <spdlog/spdlog.h>
#include "CPU/CPU_Interface.h"
#include "Memory.h"
#include "E32Image.h"
#include "Loader/E32ImageLoader.h"
#include "HLE\Kernel.h"

std::string extract_filename(const std::string& filepath)
{
	auto pos = filepath.rfind("\\");
	if (pos == std::string::npos)
		pos = -1;
	return std::string(filepath.begin() + pos + 1, filepath.end());
}

class Emulator {
public:
	
	Emulator(Memory& mem_, CPU_Interface& cpu_, std::string& app_path, std::string& lib_folder_path, std::string& rom_path) : mem(mem_), cpu(cpu_) {
		
		E32ImageLoader::parse(app_path, image);

		mem.loadRom(rom_path);
		E32ImageLoader::load(image, extract_filename(app_path), mem, lib_folder_path);

		cpu.SetPC(image.header->code_base_address + image.header->entry_point_offset); // 0x50392D54 <- entry of Euser.dll;
		//cpu.SetPC(image.header->code_base_address + image.code_section.export_directory[0]);
		//cpu.gprs[Regs::PC] = 0x5063D444; //Main of AppRun
		//cpu.cpsr.flag_T = true;

		//TODO: find the correct place where the SP is initialized
		//cpu.gprs[Regs::SP] = 0x7FFF'FFFF; //start of the ram section
		cpu.SetReg(Regs::SP, 0x7FFFFFFC); //start of the ram section aligned with last 2 bit 0


	}

	void Step() {
		try {
			switch (cpu.state) {

			case CPUState::Step:
				cpu.Step();
				cpu.state = CPUState::Stopped;
				break;

			case CPUState::Running:
				cpu.Step();
				break;
			}
		}
		catch (std::string& error_message) {
			std::cout << "Uncaught exception:\n" << error_message << std::endl;
			cpu.state = CPUState::Stopped;
			for (int i = 0; i < cpu.function_trace.size(); i++) {
				std::cout << i << " " << cpu.function_trace[i] << "\n";
			}
		}
		catch (const char* error_message) {
			std::cout << "Uncaught exception:\n" << error_message << std::endl;
			cpu.state = CPUState::Stopped;
			for (int i = 0; i < cpu.function_trace.size(); i++) {
				std::cout << i << " " << cpu.function_trace[i] << "\n";
			}
		}
	}


	CPU_Interface& cpu;
	Memory& mem;
	E32Image image;
};