#pragma once

#include "CPU/CPU_Interface.h"
#include "Memory.h"
#include "E32Image.h"


class Emulator {
public:
	
	Emulator(Memory& mem_, CPU_Interface& cpu_, std::string& app_path, std::string& lib_folder_path, std::string& rom_path);

	void Step();
	bool Equals(Emulator& that);
	void Emulator::Print();

	CPU_Interface& cpu;
	Memory& mem;
	E32Image image;
};