#pragma once
#include "../../Common.h"
#include "Decoder/IR.h"

//shoud only be 5 bits long
enum struct CpuMode : u8 {
	User =       0b10000,
	FIQ =        0b10001,
	IRQ =        0b10010,
	Supervisor = 0b10011,
	Abort =      0b10111,
	Undefined =  0b11011,
	System =     0b11111
};

struct PSR {
	bool flag_N; //bit 31
	bool flag_Z; //bit 30
	bool flag_C; //bit 29
	bool flag_V; //bit 28
	u32 reserved; //bits 27 - 8
	bool flag_inter_I; //bit 7
	bool flag_inter_F; //bit 6 
	bool flag_T; //bit 5
	CpuMode mode; //bits 4-0
};

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

	Registers(PSR& cpsr_) : gprs{}, cpsr(cpsr_) {};

	u32 operator[] (int idx) const {
		if (idx == Regs::PC) {
			//TODO: check bit 1 for non-branch thumb instr;
			return cpsr.flag_T ? gprs[idx] + 4 : gprs[idx] + 8;
		}
		else {
			return gprs[idx];
		}
	}

private:
	//Proxy element to have a differentiation getter / setter
	struct Reg {
		u32 &el;
		bool pc = false;
		bool flagT = false;

		Reg(u32 &a, bool pc_, bool flagT_) : el(a) { pc = pc_; flagT = flagT_; }
		
		operator u32() const { 
			if (pc) return flagT ? el + 4 : el + 8;
			else return el;
		}
		
		u32 operator=(u32 val) const {
			el = val;
			return val;
		}

		u32 operator=(const Reg& val) const {
			el = val.el;
			return el;
		}


		//+4/8 for pc or not ?
		/*Reg& operator+=(u32 val) {
			el += val;
			return *this;
		}

		Reg& operator-=(u32 val) {
			el -= val;
			return *this;
		}*/

	};


public:
	Reg operator[](int idx) { 
		return Reg(gprs[idx], idx == Regs::PC, cpsr.flag_T);
	}

	u32& RealPC() { return gprs[Regs::PC]; }

};