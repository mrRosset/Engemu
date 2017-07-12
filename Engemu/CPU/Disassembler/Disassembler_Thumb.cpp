#include "Disassembler.h"
#include "../Utils.h"


std::string Disassembler::Disassemble(IR_Thumb& ir) {
	
	switch (ir.instr) {

	//Branch Instructions
	case TInstructions::B_cond:  return "b" + Disassemble_Cond(ir.cond) + " " + Disassemble_Branch_Signed_Offset(SignExtend<s32>(ir.operand1 << 1, 9) + 4);
	case TInstructions::B_imm:   return "b " + Disassemble_Branch_Signed_Offset(SignExtend<s32>(ir.operand1 << 1, 12) + 4);
	case TInstructions::BL_high: return "bl high " + Disassemble_Branch_Signed_Offset((SignExtend<s32>(ir.operand1, 11) << 12) + 4);
	case TInstructions::BL:      return "bl #" + std::to_string(ir.operand1 << 1);
	case TInstructions::BX:      return "bx " + Disassemble_Reg(ir.operand1);
	
	//Data Processing Instructions
	case TInstructions::ADD_reg: return "add " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Reg(ir.operand3);
	case TInstructions::SUB_reg: return "sub " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Reg(ir.operand3);
	case TInstructions::ADD_imm: return "add " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2) + ", #" + std::to_string(ir.operand3);
	case TInstructions::SUB_imm: return "sub " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2) + ", #" + std::to_string(ir.operand3);

	case TInstructions::LSL_imm: return "lsl " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2) + ", #" + std::to_string(ir.operand3);
	case TInstructions::LSR_imm: return "lsr " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2) + ", #" + std::to_string(ir.operand3);
	case TInstructions::ASR_imm: return "asr " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2) + ", #" + std::to_string(ir.operand3);
	
	case TInstructions::MOV_imm:     return "mov " + Disassemble_Reg(ir.operand2) + ", #" + std::to_string(ir.operand1);
	case TInstructions::CMP_imm:     return "cmp " + Disassemble_Reg(ir.operand2) + ", #" + std::to_string(ir.operand1);
	case TInstructions::ADD_lar_imm: return "add " + Disassemble_Reg(ir.operand2) + ", #" + std::to_string(ir.operand1);
    case TInstructions::SUB_lar_imm: return "sub " + Disassemble_Reg(ir.operand2) + ", #" + std::to_string(ir.operand1);

	case TInstructions::AND:     return "and " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2);
	case TInstructions::EOR:     return "eor " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2);
	case TInstructions::LSL_reg: return "lsl " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2);
	case TInstructions::LSR_reg: return "lsr " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2);
	case TInstructions::ASR_reg: return "asr " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2);
	case TInstructions::ADC:     return "adc " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2);
	case TInstructions::SBC:     return "sbc " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2);
	case TInstructions::ROR:     return "ror " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2);
	case TInstructions::TST:     return "tst " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2);
	case TInstructions::NEG:     return "neg " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2);
	case TInstructions::CMP_reg: return "cmp " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2);
	case TInstructions::CMN:     return "cmn " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2);
	case TInstructions::ORR:     return "orr " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2);
	case TInstructions::MUL:     return "mul " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2);
	case TInstructions::BIC:     return "bic " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2);
	case TInstructions::MVN:     return "mvn " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2);	

	case TInstructions::ADD_imm_pc: return "add " + Disassemble_Reg(ir.operand2) + ", PC, #" + std::to_string(ir.operand1 * 4);
	case TInstructions::ADD_imm_sp: return "add " + Disassemble_Reg(ir.operand2) + ", SP, #" + std::to_string(ir.operand1 * 4);

	case TInstructions::ADD_inc_sp: return "add SP, #" + std::to_string(ir.operand1 * 4);
	case TInstructions::SUB_dec_sp: return "sub SP, #" + std::to_string(ir.operand1 * 4);
	
	case TInstructions::ADD_hig_reg: return "add " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2);
	case TInstructions::CMP_hig_reg: return "cmp " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2);
	case TInstructions::MOV_hig_reg: return "mov " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2);
	
	//Load/Store Instructions
	case TInstructions::STR_imm:  return "str "  + Disassemble_Reg(ir.operand1) + ", [" + Disassemble_Reg(ir.operand2) + ", #" + std::to_string(ir.operand3 * 4) + "]";
	case TInstructions::LDR_imm:  return "ldr "  + Disassemble_Reg(ir.operand1) + ", [" + Disassemble_Reg(ir.operand2) + ", #" + std::to_string(ir.operand3 * 4) + "]";
	case TInstructions::STRB_imm: return "strb " + Disassemble_Reg(ir.operand1) + ", [" + Disassemble_Reg(ir.operand2) + ", #" + std::to_string(ir.operand3) + "]";
	case TInstructions::LDRB_imm: return "ldrb " + Disassemble_Reg(ir.operand1) + ", [" + Disassemble_Reg(ir.operand2) + ", #" + std::to_string(ir.operand3) + "]";
	case TInstructions::STRH_imm: return "strh " + Disassemble_Reg(ir.operand1) + ", [" + Disassemble_Reg(ir.operand2) + ", #" + std::to_string(ir.operand3 * 2) + "]";
	case TInstructions::LDRH_imm: return "ldrh " + Disassemble_Reg(ir.operand1) + ", [" + Disassemble_Reg(ir.operand2) + ", #" + std::to_string(ir.operand3 * 2) + "]";
	
	case TInstructions::STR_reg:  return "str "   + Disassemble_Reg(ir.operand1) + ", [" + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Reg(ir.operand3) + "]";
	case TInstructions::STRH_reg: return "strh "  + Disassemble_Reg(ir.operand1) + ", [" + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Reg(ir.operand3) + "]";
	case TInstructions::STRB_reg: return "strb "  + Disassemble_Reg(ir.operand1) + ", [" + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Reg(ir.operand3) + "]";
	case TInstructions::LDRSB:    return "ldrsb " + Disassemble_Reg(ir.operand1) + ", [" + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Reg(ir.operand3) + "]";
	case TInstructions::LDR_reg:  return "ldr "   + Disassemble_Reg(ir.operand1) + ", [" + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Reg(ir.operand3) + "]";
	case TInstructions::LDRH_reg: return "ldrh "  + Disassemble_Reg(ir.operand1) + ", [" + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Reg(ir.operand3) + "]";
	case TInstructions::LDRB_reg: return "ldrb "  + Disassemble_Reg(ir.operand1) + ", [" + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Reg(ir.operand3) + "]";
	case TInstructions::LDRSH:    return "ldrsh " + Disassemble_Reg(ir.operand1) + ", [" + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Reg(ir.operand3) + "]";
	
	case TInstructions::LDR_pc: return "ldr " + Disassemble_Reg(ir.operand2) + ", [pc, #" + std::to_string(ir.operand1 * 4) + "]";
	case TInstructions::LDR_sp: return "ldr " + Disassemble_Reg(ir.operand2) + ", [sp, #" + std::to_string(ir.operand1 * 4) + "]";
	case TInstructions::STR_sp: return "str " + Disassemble_Reg(ir.operand2) + ", [sp, #" + std::to_string(ir.operand1 * 4) + "]";
	
	//Load/Store Multiple Instructions
	case TInstructions::LDMIA: return "ldmia " + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Small_Reg_List(ir.operand1, false, false);
	case TInstructions::STMIA: return "stmia " + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Small_Reg_List(ir.operand1, false, false);

	case TInstructions::POP:  return "pop "  + Disassemble_Small_Reg_List(ir.operand1, false, !!ir.operand2);
	case TInstructions::PUSH: return "push " + Disassemble_Small_Reg_List(ir.operand1, !!ir.operand2, false);

	//Exception Generating Instructions
	case TInstructions::SWI: return "swi #" + std::to_string(ir.operand1);
	}

	throw std::string("Could not disassemble Thumb instruction");
}

std::string Disassembler::Disassemble_Branch_Signed_Offset(s32 offset) {
	std::string sign = offset >= 0 ? "+" : "-";
	return sign + "#" + std::to_string(abs(offset));
}

std::string Disassembler::Disassemble_Small_Reg_List(u16 list, bool lr, bool pc) {
	std::string result = "{";
	bool current = false;
	unsigned start = 0;
	unsigned end = 0;

	for (int i = 0; i < 8; i++) {
		if (!current && getBit(list, i) == 1) {
			current = true;
			start = i;
		}

		if (current && getBit(list, i) == 1 && (i == 15 || getBit(list, i + 1) == 0)) {
			current = false;
			end = i;
			if (start == end) {
				result += Disassemble_Reg(start) + ",";
			}
			else {
				result += Disassemble_Reg(start) + "-" + Disassemble_Reg(end) + ",";
			}
		}
	}
	if (pc) return result + "pc}";
	else if (lr) return result + "lr}";
	else return result.substr(0, result.length() - 1) + "}";
}