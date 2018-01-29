#pragma once

#include "Gui.h"
#include "../Common.h"

class Emulator;

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
	void render_call_stack();
	void render_function_trace();

public:
	Emulator& emu;
	GuiMain(Emulator& emu, std::string& additional_title);
	bool render();
};
