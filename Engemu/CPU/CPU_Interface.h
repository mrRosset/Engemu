#pragma once

#include <functional>
#include "../Common.h"
#include "../Memory.h"
#include "Tharm/Registers.h"

enum class CPUState { Stopped, Running, Step };

class CPU_Interface {
public:
	CPUState state;
	Memory& mem;
	std::vector<std::string> call_stack;

	CPU_Interface(Memory& _mem) : mem(_mem), call_stack() {}
	virtual ~CPU_Interface() {}

	virtual void Step() = 0;
	virtual u32 GetPC() = 0;
	virtual void SetPC(u32 addr) = 0;
	virtual u32 GetReg(int index) = 0;
	virtual void SetReg(int index, u32 value) = 0;
	virtual PSR& GetCPSR() = 0;

	//Callbacks
	std::function<void(u32 number)> swi_callback = nullptr;
};