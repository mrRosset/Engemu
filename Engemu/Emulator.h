#pragma once

#include <spdlog/spdlog.h>
#include "E32Image.h"
#include "HLE/Kernel.h"
#include "CPU/CPU_Interface.h"
#include "Memory/Memory_Interface.h"
#include "Loader/E32ImageLoader.h"

class Emulator {
public:
	
	Emulator(Memory_Interface& mem_, CPU_Interface& cpu_, std::string& app_path, std::string& lib_folder_path, std::string& rom_path);

	void Step();
	CPU_Interface& getCPU();
	void setCPU(CPU_Interface& cpu);

	CPUState state;
private:
	CPU_Interface* cpu;
	Memory_Interface& mem;
	E32Image image;
};