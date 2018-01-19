#pragma once
#include "../../Common.h"
#include "IR.h"

namespace Decoder {
	//ARM
	void Decode(IR_ARM& ir, u32 instr);
	void Decode_Data_Processing(IR_ARM& ir, u32 instr);
	void Decode_Shifter_operand(IR_ARM& ir, u32 instr);
	void Decode_Branch(IR_ARM& ir, u32 instr);
	void Decode_Multiply(IR_ARM& ir, u32 instr);
	void Decode_Status_Register(IR_ARM& ir, u32 instr);
	void Decode_Load_Store_W_UB(IR_ARM& ir, u32 instr);
	void Decode_Load_Store_H_SB(IR_ARM& ir, u32 instr);
	void Decode_Load_Store_Multiple(IR_ARM& ir, u32 instr);
	void Decode_Semaphore(IR_ARM& ir, u32 instr);
	void Decode_Exception_Generating(IR_ARM& ir, u32 instr);
	void Decode_Coprocessor(IR_ARM& ir, u32 instr);

	//THUMB
	void Decode(IR_Thumb& ir, u16 instr);
	void Decode_Conditional_Branch(IR_Thumb& ir, u16 instr);
	void Decode_Unconditionnal_Branch(IR_Thumb& ir, u16 instr);
	void Decode_Branch_With_Exchange(IR_Thumb& ir, u16 instr);
	void Decode_Add_Sub_reg_imm(IR_Thumb& ir, u16 instr);
	void Decode_Shift_Imm(IR_Thumb &ir, u16 instr);
	void Decode_Add_Sub_Mov_Cmp_imm(IR_Thumb& ir, u16 instr);
	void Decode_Data_Processing_Register(IR_Thumb& ir, u16 instr);
	void Decode_Add_To_PC_SP(IR_Thumb& ir, u16 instr);
	void Decode_Adjust_SP(IR_Thumb& ir, u16 instr);
	void Decode_Special_Data_Processing(IR_Thumb& ir, u16 instr);
	void Decoder_Load_Store_W_B_H_imm(IR_Thumb& ir, u16 instr);
	void Decode_Load_Store_Reg_offset(IR_Thumb& ir, u16 instr);
	void Decode_Load_PC(IR_Thumb& ir, u16 instr);
	void Decode_Load_Store_SP(IR_Thumb& ir, u16 instr);
	void Decode_Load_Store_Multiple(IR_Thumb& ir, u16 instr);
	void Decode_Push_Pop(IR_Thumb& ir, u16 instr);
	void Decode_Exception_Generating(IR_Thumb& ir, u16 instr);
}