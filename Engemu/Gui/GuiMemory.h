#pragma once

#include "Gui.h"
#include "../Common.h"
#include "MemoryEditor.h"


class GageMemory;

class GuiMemory : public Gui
{
private:
	MemoryEditor memory_editor;

public:
	GageMemory& mem;
	GuiMemory(GageMemory& mem, std::string& additional_title);
	bool render();
};
