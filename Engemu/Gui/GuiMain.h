#pragma once

#include "Gui.h"

class CPU;

class GuiMain : public Gui
{
private:
	bool show_cpu_window = true;
	bool track_pc = true;
	void render_cpu();

public:
	CPU& cpu;
	GuiMain(CPU& cpu, std::string& additional_title);
	bool render();
};
