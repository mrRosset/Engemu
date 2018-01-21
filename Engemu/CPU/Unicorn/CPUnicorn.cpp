#include "CPUnicorn.h"
#include <unicorn/arm.h>
#include "../../Common.h"
#include "../Tharm/Utils.h"
#include "../Tharm/Decoder/Decoder.h"

#define CHECKED(expr)                                                                              \
    do {                                                                                           \
        if (auto _cerr = (expr)) {                                                                 \
			throw std::string("Failure with error: ") + std::string(uc_strerror(_cerr));           \
        }                                                                                          \
    } while (0)

static void hook_block(uc_engine *uc, uint64_t address, uint32_t size, void *user_data)
{
	auto cpu = (CPUnicorn*)user_data;
	cpu->logger->info("Tracing basic block at {}, block size = {}", address, size);
}

static void hook_code(uc_engine *uc, uint64_t address, uint32_t size, void *user_data)
{
	auto cpu = (CPUnicorn*)user_data;
	cpu->logger->info("Tracing instruction at {}, instruction size = {}", address, size);
}

static void InterruptHook(uc_engine* uc, u32 intNo, void* user_data) {
	auto cpu = (CPUnicorn*)user_data;
	cpu->logger->info("InterruptHook: intNo: {}", intNo);
	//TODO Handle other interrupts than SWI
	//TODO Handle Thumb SWI
	IR_ARM ir;
	u32 instr = cpu->mem.read32(cpu->GetPC()-4);
	Decoder::Decode(ir, instr);
	cpu->swi_callback(ir.operand1);
}

static bool UnmappedMemoryHook(uc_engine* uc, uc_mem_type type, u64 addr, int size, u64 value,	void* user_data) {
	throw std::string("Attempted to read from unmapped memory") + std::to_string(addr);
	return false;
}

CPUnicorn::CPUnicorn(GageMemory& mem_) : CPU_Interface(mem_) {
	logger = spdlog::get("console");
	CHECKED(uc_open(UC_ARCH_ARM, UC_MODE_ARM, &uc));

	uc_hook hook{};
	CHECKED(uc_hook_add(uc, &hook, UC_HOOK_INTR, (void*)InterruptHook, this, 0, -1));
	CHECKED(uc_hook_add(uc, &hook, UC_HOOK_MEM_INVALID, (void*)UnmappedMemoryHook, this, 0, -1));
	//CHECKED(uc_hook_add(uc, &hook, UC_HOOK_BLOCK, hook_block, this, 1, 0));
	//CHECKED(uc_hook_add(uc, &hook, UC_HOOK_CODE, hook_code, this, 0, -1));

	CHECKED(uc_mem_map_ptr(uc, 0x0040'0000, mem_.user_data.size(), UC_PROT_READ | UC_PROT_WRITE | UC_PROT_EXEC, mem_.user_data.data()));
	CHECKED(uc_mem_map_ptr(uc, 0x5000'0000, mem_.rom.size(), UC_PROT_READ | UC_PROT_WRITE | UC_PROT_EXEC, mem_.rom.data()));
	CHECKED(uc_mem_map_ptr(uc, 0x6000'0000, mem_.ram.size(), UC_PROT_READ | UC_PROT_WRITE | UC_PROT_EXEC, mem_.ram.data()));
}

CPUnicorn::~CPUnicorn() {
	uc_close(uc);
}

void CPUnicorn::Step() {
	auto cpsr = GetCPSR();
	if (cpsr.flag_T) {
		CHECKED(uc_emu_start(uc, GetPC() | 1, (1ULL << 32) | 1, 0, 1));
	}
	else {
		CHECKED(uc_emu_start(uc, GetPC(), 1ULL << 32, 0, 1));
	}
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
	CHECKED(uc_reg_write(uc, treg, &val));
}

PSR& CPUnicorn::GetCPSR() {
	u32 cpsr{};
	CHECKED(uc_reg_read(uc, UC_ARM_REG_CPSR, &cpsr));
	u32_to_PSR(cpsr, temp);
	return temp;
}