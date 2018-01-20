#include "CPUnicorn.h"
#include <unicorn/arm.h>
#include "../../Common.h"

#define CHECKED(expr)                                                                              \
    do {                                                                                           \
        if (auto _cerr = (expr)) {                                                                 \
			throw std::string("Failure with error") + std::string(uc_strerror(_cerr));             \
        }                                                                                          \
    } while (0)


static void InterruptHook(uc_engine* uc, u32 intNo, void* user_data) {
	//TODO
}

static bool UnmappedMemoryHook(uc_engine* uc, uc_mem_type type, u64 addr, int size, u64 value,	void* user_data) {
	throw std::string("Attempted to read from unmapped memory") + std::to_string(addr);
	return false;
}

CPUnicorn::CPUnicorn(GageMemory& mem_) : CPU_Interface(mem_) {
	CHECKED(uc_open(UC_ARCH_ARM, UC_MODE_ARM, &uc));

	uc_hook hook{};
	CHECKED(uc_hook_add(uc, &hook, UC_HOOK_INTR, (void*)InterruptHook, this, 0, -1));
	CHECKED(uc_hook_add(uc, &hook, UC_HOOK_MEM_INVALID, (void*)UnmappedMemoryHook, this, 0, -1));

	uc_mem_map(uc, 0x10000, 2 * 1024 * 1024, UC_PROT_ALL);
}

CPUnicorn::~CPUnicorn() {
	uc_close(uc);
}

void CPUnicorn::Step() {
	CHECKED(uc_emu_start(uc, GetPC(), 1ULL << 32, 0, 1));
}

u32 CPUnicorn::GetPC() {
	u32 val{};
	CHECKED(uc_reg_read(uc, UC_ARM_REG_PC, &val));
	return val;
}

void CPUnicorn::SetPC(u32 pc) {
	CHECKED(uc_reg_write(uc, UC_ARM_REG_PC, &pc));
}

u32 CPUnicorn::GetReg(int index) {
	u32 val{};
	auto treg = UC_ARM_REG_SP;
	
	switch (index) {
	case 0: treg = UC_ARM_REG_R0; break;
	case 1: treg = UC_ARM_REG_R1; break;
	case 2: treg = UC_ARM_REG_R2; break;
	case 3: treg = UC_ARM_REG_R3; break;
	case 4: treg = UC_ARM_REG_R4; break;
	case 5: treg = UC_ARM_REG_R5; break;
	case 6: treg = UC_ARM_REG_R6; break;
	case 7: treg = UC_ARM_REG_R7; break;
	case 8: treg = UC_ARM_REG_R8; break;
	case 9: treg = UC_ARM_REG_R9; break;
	case 10: treg = UC_ARM_REG_R10; break;
	case 11: treg = UC_ARM_REG_R11; break;
	case 12: treg = UC_ARM_REG_R12; break;
	case 13: treg = UC_ARM_REG_R13; break;
	case 14: treg = UC_ARM_REG_R14; break;
	case 15: treg = UC_ARM_REG_R15; break;
	}
	
	CHECKED(uc_reg_read(uc, treg, &val));
	return val;
}

void CPUnicorn::SetReg(int index, u32 val) {
	auto treg = UC_ARM_REG_SP;
	switch (index) {
	case 0: treg = UC_ARM_REG_R0; break;
	case 1: treg = UC_ARM_REG_R1; break;
	case 2: treg = UC_ARM_REG_R2; break;
	case 3: treg = UC_ARM_REG_R3; break;
	case 4: treg = UC_ARM_REG_R4; break;
	case 5: treg = UC_ARM_REG_R5; break;
	case 6: treg = UC_ARM_REG_R6; break;
	case 7: treg = UC_ARM_REG_R7; break;
	case 8: treg = UC_ARM_REG_R8; break;
	case 9: treg = UC_ARM_REG_R9; break;
	case 10: treg = UC_ARM_REG_R10; break;
	case 11: treg = UC_ARM_REG_R11; break;
	case 12: treg = UC_ARM_REG_R12; break;
	case 13: treg = UC_ARM_REG_R13; break;
	case 14: treg = UC_ARM_REG_R14; break;
	case 15: treg = UC_ARM_REG_R15; break;
	}
	CHECKED(uc_reg_write(uc, UC_ARM_REG_SP, &val));
}

PSR& CPUnicorn::GetCPSR() {
	return temp;
}