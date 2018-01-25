#pragma once

#include "../Common.h"
#include "../Gui/GuiMain.h"

class CPU_Interface;
typedef u32 vir_add;

namespace Kernel {
	extern vir_add RHeap_ptr;
	extern vir_add TrapHandler_ptr;
	
	void Executive_Call(u32 number, CPU_Interface& cpu, GuiMain* gui);

	void User_Heap(CPU_Interface& cpu, GuiMain* gui);
	void User_LockedDec(CPU_Interface& cpu);
	void User_LockedInc(CPU_Interface& cpu);
	void RSemaphore_Wait(CPU_Interface& cpu);
	void RProcess_CommandLineLength(CPU_Interface& cpu);
	void TChar_GetCategory(CPU_Interface& cpu);
	void TChar_GetUpperCase(CPU_Interface& cpu);
	void User_SetTrapHandler(CPU_Interface& cpu);
	void UserSvr__InitRegisterCallback(CPU_Interface& cpu);
	void User__WaitForAnyRequest(CPU_Interface& cpu);
}