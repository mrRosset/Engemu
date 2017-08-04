#pragma once
#include <tuple>
#include <functional>
#include "../Common.h"
#include "Memory.h"

//TODO: make sure scope of the elements are appropriate

//shoud only be 5 bits long
enum struct CpuMode: u8 {
	User = 0b10000,
	FIQ = 0b10001,
	IRQ = 0b10010,
	Supervisor = 0b10011,
	Abort = 0b10111,
	Undefined = 0b11011,
	System = 0b11111
};


//TODO: Think about default values.
struct PSR {
	bool flag_N, flag_Z, flag_C, flag_V; //bits 31 - 28
	u32 reserved; //bits 27 - 8
	bool flag_inter_I, flag_inter_F; //bits 7-6 
	bool flag_T; //bit 5
	CpuMode mode; //bits 4-0
};

//Forward declaration
struct IR_ARM;
struct IR_Thumb;
struct Shifter_op;
enum class Conditions : u8;

class CPU {
public:

	enum class State { Stopped, Running, Step };
	
	//TODO: Take care of registers when in mode switch.

	/*
	ARM has 31 general-purpose 32-bit registers. At any one time, 16 of these registers are visible. The other
	registers are used to speed up exception processing. All the register specifiers in ARM instructions can
	address any of the 16 visible registers.
	-> which one is the 31th ? Can only see 30
	*/
	//use u32 or s32 ? -> mgba use s32
	u32 gprs[16];

	PSR cpsr;
	PSR spsr;
	Memory mem;

	State state;

	CPU();
	void Step();
	bool Check_Condition(Conditions& cond);

	//ARM
	void Execute(IR_ARM& ir);
	void Exception_Generating(IR_ARM& ir);
	void Load_Store_Multiple(IR_ARM& ir);
	void Load_Store(IR_ARM& ir);
	void Status_Register_Access(IR_ARM& ir);
	void Multiply(IR_ARM& ir);
	void MUL_Instr1(bool S, unsigned Rd, u32 result);
	void MUL_Instr2(bool S, unsigned RdHi, unsigned RdLo, u32 resultHi, u32 resultLo);
	void Branch(IR_ARM& ir);
	void Data_Processing(IR_ARM& ir);
	std::tuple<u32, bool> shifter_operand(Shifter_op& so, bool negatif);

	//Thumb
	void Execute(IR_Thumb& ir);
	void Load_Store_Multiple(IR_Thumb& ir);
	void Load_Store(IR_Thumb& ir);
	void Branch(IR_Thumb& ir);
	void Data_Processing_1_2(IR_Thumb& ir);
	void Data_Processing_3(IR_Thumb& ir);
	void Data_Processing_4(IR_Thumb& ir);
	void Data_Processing_5(IR_Thumb& ir);
	void Data_Processing_6_7(IR_Thumb& ir);
	void Data_Processing_8(IR_Thumb& ir);

	//Common
	void DP_Instr1(bool S, unsigned Rd, u32 result, std::function<bool()> N, std::function<bool()> Z, std::function<bool()> C, std::function<bool()> V);
	void DP_Instr1(unsigned Rd, u32 result, std::function<bool()> N, std::function<bool()> Z, std::function<bool()> C, std::function<bool()> V);
	void DP_Instr2(u32 result, std::function<bool(u32)> N, std::function<bool(u32)> Z, std::function<bool(u32)> C, std::function<bool(u32)> V);
	
	//useful lambdas
	std::function<bool()> fun_C = [&]()->bool {return cpsr.flag_C; };
	std::function<bool()> fun_V = [&]()->bool {return cpsr.flag_V; };
	std::function<bool(u32)> fun_r_31 = [&](u32 r)->bool {return !!((r >> 31) & 0b1); };
	std::function<bool(u32)> fun_r_0 = [&](u32 r)->bool {return r == 0; };
	std::function<bool(u32)> fun_r_C = [&](u32 r)->bool {return cpsr.flag_C; };
	std::function<bool(u32)> fun_r_V = [&](u32 r)->bool {return cpsr.flag_V; };

	//Call stack
	std::vector<u32> call_stack;

	//Callbacks
	std::function<void(u32 number)> swi_callback = nullptr;
};
