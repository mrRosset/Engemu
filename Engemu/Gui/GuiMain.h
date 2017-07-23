#pragma once

#include <unordered_map>
#include "Gui.h"
#include "../Common.h"

class CPU;

class GuiMain : public Gui
{
private:
	bool show_cpu_window = true;
	bool track_pc = true;
	void render_cpu();
	void render_memoryNav();
	bool render_controls();
	void render_disassembly(bool scroll_to_pc);
	void render_registers();
	void render_stack();
	std::unordered_map<u32, std::string> symbols;

public:
	CPU& cpu;
	GuiMain(CPU& cpu, std::string& additional_title);
	bool render();
	void loadSymbols(std::string& symbol_file);
};
