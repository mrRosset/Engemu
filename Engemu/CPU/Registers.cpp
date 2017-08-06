#include "Registers.h"
#include "CPU.h"
#include "Decoder\IR.h"

Registers::Registers(PSR& cpsr_) : gprs{}, cpsr(cpsr_) {}

// Is this the only getter ?
u32 Registers::operator[] (int idx) const {
	if (idx == Regs::PC) {
		//TODO: check bit 1 for non-branch thumb instr;
		return cpsr.flag_T ? gprs[idx] + 4 : gprs[idx] + 8;
	}
	else {
		return gprs[idx];
	}
}

// Is this always used a setter ?
u32& Registers::operator[](int idx) { 
	return gprs[idx];
}