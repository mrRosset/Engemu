#pragma once

#include "../Common.h"

class CPU_Interface {
public:
	virtual ~CPU_Interface() {}

	virtual void Step() = 0;
	virtual u32 GetPC() = 0;
	virtual void SetPC(u32 addr) = 0;
	virtual u32 GetReg(int index) = 0;
	virtual void SetReg(int index, u32 value) = 0;
	virtual u32 GetCPSR() = 0;
	virtual void SetCPSR(u32 cpsr) = 0;

};