#pragma once

#include "../Common.h"
#include "../Gui/GuiMain.h"

class CPU;
typedef u32 vir_add;

namespace Kernel {
	extern vir_add RHeap_ptr;
	
	void Executive_Call(u32 number, CPU& cpu, GuiMain* gui);

	void User_Heap(CPU& cpu, GuiMain* gui);
	void User_LockedDec(CPU& cpu);
	void User_LockedInc(CPU& cpu);
	void RSemaphore_Wait(CPU& cpu);
	void RProcess_CommandLineLength(CPU& cpu);
	void TChar_GetCategory(CPU& cpu);
	void TChar_GetUpperCase(CPU& cpu);
}