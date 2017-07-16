#pragma once

#include "../Common.h"
#include "../Gui/Gui.h"

class CPU;

namespace Kernel {
	void Executive_Call(u32 number, CPU& cpu, Gui* gui);

}