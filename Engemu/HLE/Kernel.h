#pragma once

#include "..\Common.h"

class CPU;

namespace Kernel {
	void Executive_Call(CPU& cpu, u32 number);
}