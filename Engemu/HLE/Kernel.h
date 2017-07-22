#pragma once

#include "../Common.h"
#include "../Gui/Gui.h"

class CPU;
typedef u32 vir_add;

namespace Kernel {
	extern vir_add RHeap_ptr;
	
	void Executive_Call(u32 number, CPU& cpu, Gui* gui);

	void User_Heap(CPU & cpu, Gui * gui);

}