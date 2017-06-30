#pragma once

#include "Common.h"

enum class CPUType : u32 {
	x86 = 0x1000,
	Arm = 0x2000,
	Mcore = 0x4000
};

enum class ProcessPriority
{
	Low = 150,
	Background = 250,
	Foreground = 350,
	High = 450,
	WindowServer = 650,
	FileServer = 750,
	RealTimeServer = 850,
	Supervisor = 950
};
