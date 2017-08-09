#pragma once

#include "Gui.h"
#include "../Common.h"
#include "MemoryEditor.h"


class Memory;

class GuiMemory : public Gui
{
private:
	MemoryEditor memory_editor;

public:
	Memory& mem;
	GuiMemory(Memory& mem, std::string& additional_title);
	bool render();
};
