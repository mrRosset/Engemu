#pragma once
#include <tuple>
#include <functional>
#include <vector>
#include "../../Common.h"
#include "../../Memory/Memory_Interface.h"
#include "../CPU_Interface.h"
#include "Registers.h"


//Forward declaration
struct IR_ARM;
struct IR_Thumb;
struct Shifter_op;
enum class Conditions : u8;

class CPU : public CPU_Interface {
public:

	PSR cpsr;
	PSR spsr;
	Registers gprs;

	CPU(Memory_Interface& mem);

	void ExecuteNextInstruction() override;
	u32 GetPC() override;
	void SetPC(u32 addr) override;
	u32 GetReg(int index) override;
	void SetReg(int index, u32 value) override;
	PSR& GetCPSR() override;
	void SetCPSR(PSR& cpsr) override;
	
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


};
