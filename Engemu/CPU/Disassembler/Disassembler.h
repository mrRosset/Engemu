#pragma once

#include <string>
#include "../../Common.h"
#include "../Decoder/IR.h"


namespace Disassembler {
	//ARM
	std::string Disassemble(IR_ARM& ir);
	std::string Disassemble_Reg(u32 reg);
	std::string Disassemble_Cond(Conditions& cond);
	std::string Disassemble_Shifter_Operand(Shifter_op & so, bool negatif);
	std::string Disassemble_Shifter_Operand(Shifter_op & so);
	std::string Disassemble_Branch_Offset(u32 operand);
	std::string Disassemble_PSR(u32 R);
	std::string Disassemble_Fields(u32 fields);
	std::string Disassemble_LS_Shifter_Operand(IR_ARM& ir);
	std::string Disassemble_Load_Store_Multiple(IR_ARM& ir);
	std::string Disassemble_Reg_List(u32 list);

	//THUMB
	std::string Disassemble(IR_Thumb& ir);
	std::string Disassemble_Signed_Offset(s16 offset);
}