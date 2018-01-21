#pragma once

#include <unicorn/unicorn.h>
#include <spdlog/spdlog.h>
#include "../CPU_Interface.h"

class CPUnicorn : public CPU_Interface {
public:
	std::shared_ptr<spdlog::logger> logger;

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
	PSR temp{};
};