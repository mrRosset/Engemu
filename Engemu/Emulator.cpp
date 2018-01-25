#include "Emulator.h"
#include "Loader/E32ImageLoader.h"

std::string extract_name(const std::string& filepath)
{
	auto pos = filepath.rfind("\\");
	if (pos == std::string::npos)
		pos = -1;
	return std::string(filepath.begin() + pos + 1, filepath.end());
}

Emulator::Emulator(Memory & mem_, CPU_Interface & cpu_, std::string & app_path, std::string & lib_folder_path, std::string & rom_path) : mem(mem_), cpu(cpu_) {

	E32ImageLoader::parse(app_path, image);

	mem.loadRom(rom_path);
	E32ImageLoader::load(image, extract_name(app_path), mem, lib_folder_path);

	cpu.SetPC(image.header->code_base_address + image.header->entry_point_offset); // 0x50392D54 <- entry of Euser.dll;
	//cpu.SetPC(image.header->code_base_address + image.code_section.export_directory[0]);
	//cpu.gprs[Regs::PC] = 0x5063D444; //Main of AppRun
	//cpu.cpsr.flag_T = true;

	//TODO: find the correct place where the SP is initialized
	//cpu.gprs[Regs::SP] = 0x7FFF'FFFF; //start of the ram section
	cpu.SetReg(Regs::SP, 0x7FFFFFFC); //start of the ram section aligned with last 2 bit 0


}

void Emulator::Step() {
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

bool Emulator::Equals(Emulator & that) {
	CPU_Interface& cpu2 = that.cpu;
	PSR& one = cpu.GetCPSR();
	PSR& two = that.cpu.GetCPSR();

	return cpu.GetReg(0) == cpu2.GetReg(0) &&
		cpu.GetReg(1) == cpu2.GetReg(1) &&
		cpu.GetReg(2) == cpu2.GetReg(2) &&
		cpu.GetReg(3) == cpu2.GetReg(3) &&
		cpu.GetReg(4) == cpu2.GetReg(4) &&
		cpu.GetReg(5) == cpu2.GetReg(5) &&
		cpu.GetReg(6) == cpu2.GetReg(6) &&
		cpu.GetReg(7) == cpu2.GetReg(7) &&
		cpu.GetReg(8) == cpu2.GetReg(8) &&
		cpu.GetReg(9) == cpu2.GetReg(9) &&
		cpu.GetReg(10) == cpu2.GetReg(10) &&
		cpu.GetReg(11) == cpu2.GetReg(11) &&
		cpu.GetReg(12) == cpu2.GetReg(12) &&
		cpu.GetReg(13) == cpu2.GetReg(13) &&
		cpu.GetReg(14) == cpu2.GetReg(14) &&
		cpu.GetPC() == cpu2.GetPC() &&
		one.flag_C == two.flag_C &&
		one.flag_N == two.flag_N &&
		one.flag_T == two.flag_T &&
		one.flag_V == two.flag_V &&
		one.flag_Z == two.flag_Z;
}
