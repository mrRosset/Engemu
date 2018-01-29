#pragma once

#include <spdlog/spdlog.h>
#include "CPU/CPU_Interface.h"
#include "Memory.h"
#include "E32Image.h"
#include "Loader/E32ImageLoader.h"
#include "HLE\Kernel.h"

class Emulator {
public:
	
	Emulator(Memory& mem_, CPU_Interface& cpu_, std::string& app_path, std::string& lib_folder_path, std::string& rom_path);

	void Step();
	CPU_Interface& getCPU();
	void setCPU(CPU_Interface& cpu);

private:
	CPU_Interface* cpu;
	Memory& mem;
	E32Image image;
};