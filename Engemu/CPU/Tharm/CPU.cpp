#include "CPU.h"
#include "Utils.h"
#include "Decoder/Decoder.h"

/*
Code in this file is compiler-specific.
the operator >> is used. This operator is can be logical or arithmetic
In VC++ it's arithmetic. https://msdn.microsoft.com/en-us/library/336xbhcz.aspx
Not tested in any other compiler,
*/

CPU::CPU(Memory& mem_) : cpsr{}, spsr{}, gprs(cpsr), CPU_Interface(mem_) {
	//TODO: Seriously think about initialization values
	cpsr.flag_N = cpsr.flag_Z = cpsr.flag_C = cpsr.flag_V = cpsr.flag_inter_I = cpsr.flag_inter_F = cpsr.flag_T = false;
	cpsr.reserved = 0;
	cpsr.mode = CpuMode::User;
	cpsr.flag_inter_I = true;
	cpsr.flag_inter_F = true;
}

void CPU::ExecuteNextInstruction() {
	if (cpsr.flag_T) {
		u16 instr = mem.read16(gprs.RealPC());
		IR_Thumb ir;
		Decoder::Decode(ir, instr);

		gprs.RealPC() += 2;

		if (Check_Condition(ir.cond)) {
			Execute(ir);
		}

	}
	else {
		u32 instr = mem.read32(gprs.RealPC());
		IR_ARM ir;
		Decoder::Decode(ir, instr);

		gprs.RealPC() += 4;

		if (Check_Condition(ir.cond)) {
			Execute(ir);
		}
	}
	

	// interrupts and prefetch aborts exist whether or not conditions are met 
}

u32 CPU::GetPC() {
	return gprs.RealPC();
}

void CPU::SetPC(u32 addr) {
	gprs.RealPC() = addr;
}

u32 CPU::GetReg(int index) {
	return gprs[index];
}

void CPU::SetReg(int index, u32 value) {
	gprs[index] = value;
}

PSR& CPU::GetCPSR() {
	return cpsr;
}

bool CPU::Check_Condition(Conditions& cond) {
	switch (cond) {
	case Conditions::EQ: return cpsr.flag_Z;
	case Conditions::NE: return !cpsr.flag_Z;
	case Conditions::CS_HS: return cpsr.flag_C;
	case Conditions::CC_LO: return !cpsr.flag_C;
	case Conditions::MI: return cpsr.flag_N;
	case Conditions::PL: return !cpsr.flag_N;
	case Conditions::VS: return cpsr.flag_V;
	case Conditions::VC: return !cpsr.flag_V;
	case Conditions::HI: return cpsr.flag_C && !cpsr.flag_Z;
	case Conditions::LS: return !cpsr.flag_C || cpsr.flag_Z;
	case Conditions::GE: return cpsr.flag_N == cpsr.flag_V;
	case Conditions::LT: return cpsr.flag_N != cpsr.flag_V;
	case Conditions::GT: return !cpsr.flag_Z && cpsr.flag_N == cpsr.flag_V;
	case Conditions::LE: return cpsr.flag_Z || cpsr.flag_N != cpsr.flag_V;
	case Conditions::AL: return true;
	case Conditions::NV: throw std::string("Unpredictable instructions are not emulated");
	}
	throw "Invalid condition";
}

void CPU::DP_Instr1(bool S, unsigned Rd, u32 result, std::function<bool()> N, std::function<bool()> Z, std::function<bool()> C, std::function<bool()> V) {
	if (S && Rd != Regs::PC) {
		//Assigning those before Rd because of the cases where Rn == Rd.
		//Since those depends on the value of Rn, it might get overwritten afterward.
		cpsr.flag_C = C();
		cpsr.flag_V = V();
	}
	gprs[Rd] = result;
	if (S && Rd == Regs::PC) {
		throw("no sprs in user/system mode, other mode not implemented yet");
	}
	else if (S) {
		cpsr.flag_N = N();
		cpsr.flag_Z = Z();
	}
}

void CPU::DP_Instr1(unsigned Rd, u32 result, std::function<bool()> N, std::function<bool()> Z, std::function<bool()> C, std::function<bool()> V) {
	//Assigning those before Rd because of the cases where Rn == Rd.
	//Since those depends on the value of Rn, it might get overwritten afterward.
	cpsr.flag_C = C();
	cpsr.flag_V = V();
	gprs[Rd] = result;
	cpsr.flag_N = N();
	cpsr.flag_Z = Z();
}

void CPU::DP_Instr2(u32 result, std::function<bool(u32)> N, std::function<bool(u32)> Z, std::function<bool(u32)> C, std::function<bool(u32)> V) {
	//TODO: What is alu_out ? what's supposed to happens to the result ?
	u32 alu_out = result;
	cpsr.flag_N = N(result);
	cpsr.flag_Z = Z(result);
	cpsr.flag_C = C(result);
	cpsr.flag_V = V(result);
}