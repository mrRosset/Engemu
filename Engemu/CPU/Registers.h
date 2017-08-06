#pragma once
#include "../Common.h"

struct PSR;

class Registers {

	//TODO: Take care of registers when in mode switch.

	/*
	ARM has 31 general-purpose 32-bit registers. At any one time, 16 of these registers are visible. The other
	registers are used to speed up exception processing. All the register specifiers in ARM instructions can
	address any of the 16 visible registers.
	-> which one is the 31th ? Can only see 30
	*/
	//use u32 or s32 ? -> mgba use s32
	u32 gprs[16];

	PSR& cpsr;

public:

	Registers(PSR& cpsr);

	u32 operator[] (int idx) const;
	u32& operator[](int idx);

};