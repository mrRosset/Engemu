#pragma once

#include <unicorn/unicorn.h>
#include "../CPU_Interface.h"

class CPUnicorn : public CPU_Interface {
public:

	CPUnicorn(GageMemory& mem);
	~CPUnicorn();
	void Step() override;
	u32 GetPC() override;
	void SetPC(u32 addr) override;
	u32 GetReg(int index) override;
	void SetReg(int index, u32 value) override;
	PSR& GetCPSR() override;

private:
	uc_engine * uc{};
};