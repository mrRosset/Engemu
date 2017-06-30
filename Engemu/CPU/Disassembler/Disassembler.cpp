#include <cstdlib>
#include "Disassembler.h"
#include "../Utils.h"



std::string Disassembler::Disassemble(IR_ARM & ir) {
	std::string s = ir.s ? "s" : "";

	switch (ir.instr) {

	//Data Processing Instructions
	case AInstructions::TST: return "tst" + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Shifter_Operand(ir.shifter_operand);
	case AInstructions::TEQ: return "teq" + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Shifter_Operand(ir.shifter_operand);
	case AInstructions::CMP: return "cmp" + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Shifter_Operand(ir.shifter_operand);
	case AInstructions::CMN: return "cmn" + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Shifter_Operand(ir.shifter_operand);
	case AInstructions::MOV: return "mov" + s + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Shifter_Operand(ir.shifter_operand);
	case AInstructions::MVN: return "mvn" + s + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Shifter_Operand(ir.shifter_operand);
	case AInstructions::AND: return "and" + s + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Shifter_Operand(ir.shifter_operand);
	case AInstructions::EOR: return "eor" + s + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Shifter_Operand(ir.shifter_operand);
	case AInstructions::SUB: return "sub" + s + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Shifter_Operand(ir.shifter_operand);
	case AInstructions::RSB: return "rsb" + s + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Shifter_Operand(ir.shifter_operand);
	case AInstructions::ADD: return "add" + s + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Shifter_Operand(ir.shifter_operand);
	case AInstructions::ADC: return "adc" + s + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Shifter_Operand(ir.shifter_operand);
	case AInstructions::SBC: return "sbc" + s + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Shifter_Operand(ir.shifter_operand);
	case AInstructions::RSC: return "rsc" + s + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Shifter_Operand(ir.shifter_operand);
	case AInstructions::ORR: return "orr" + s + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Shifter_Operand(ir.shifter_operand);
	case AInstructions::BIC: return "bic" + s + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Shifter_Operand(ir.shifter_operand);

	//Branch Instructions
	case AInstructions::B: return "b" + Disassemble_Cond(ir) + " " + Disassemble_Branch_Offset(ir.operand1);
	case AInstructions::BL: return "bl" + Disassemble_Cond(ir) + " " + Disassemble_Branch_Offset(ir.operand1) ;
	case AInstructions::BX: return "bx" + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand1);

	//Multiply Instructions
	case AInstructions::MUL: return "mul" + s + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand4) + ", " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2); break;
	case AInstructions::MLA: return "mla" + s + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand4) + ", " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Reg(ir.operand3); break;
	case AInstructions::UMULL: return "umull" + s + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand3) + ", " + Disassemble_Reg(ir.operand4) + ", " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2); break;
	case AInstructions::UMLAL: return "umlal" + s + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand3) + ", " + Disassemble_Reg(ir.operand4) + ", " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2); break;
	case AInstructions::SMULL: return "smull" + s + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand3) + ", " + Disassemble_Reg(ir.operand4) + ", " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2); break;
	case AInstructions::SMLAL: return "smlal" + s + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand3) + ", " + Disassemble_Reg(ir.operand4) + ", " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_Reg(ir.operand2); break;

	//Status Register Access Instructions
	case AInstructions::MRS: return "mrs" + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand2) + ", " + Disassemble_PSR(ir.operand1); break;
	case AInstructions::MSR: return "msr" + Disassemble_Cond(ir) + " " + Disassemble_PSR(ir.operand1) + "_" + Disassemble_Fields(ir.operand2) + ", " + Disassemble_Shifter_Operand(ir.shifter_operand); break;
	
	//Load/Store unsigned byte/word
	case AInstructions::LDR: return "ldr" + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_LS_Shifter_Operand(ir); break;
	case AInstructions::STR: return "str" + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_LS_Shifter_Operand(ir); break;
	case AInstructions::LDRT: return "ldrt" + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_LS_Shifter_Operand(ir); break;
	case AInstructions::STRT: return "strt" + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_LS_Shifter_Operand(ir); break;
	case AInstructions::LDRB: return "ldrb" + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_LS_Shifter_Operand(ir); break;
	case AInstructions::STRB: return "strb" + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_LS_Shifter_Operand(ir); break;
	case AInstructions::LDRBT: return "ldrbt" + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_LS_Shifter_Operand(ir); break;
	case AInstructions::STRBT: return "strbt" + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand1) + ", *" + Disassemble_LS_Shifter_Operand(ir); break;
	
	//Load/Store signed byte / halfword
	case AInstructions::LDRH: return "ldrh" + Disassemble_Cond(ir) +" " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_LS_Shifter_Operand(ir); break;
	case AInstructions::LDRSH: return "ldrsh" + Disassemble_Cond(ir) +" " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_LS_Shifter_Operand(ir); break;
	case AInstructions::LDRSB: return "ldrsb" + Disassemble_Cond(ir) +" " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_LS_Shifter_Operand(ir); break;
	case AInstructions::STRH: return "strh" + Disassemble_Cond(ir) +" " + Disassemble_Reg(ir.operand1) + ", " + Disassemble_LS_Shifter_Operand(ir); break;
	
	//Load/Store multiple
	case AInstructions::LDMIA: return "ldmia" + Disassemble_Cond(ir) + " " + Disassemble_Load_Store_Multiple(ir); break;
	case AInstructions::LDMIB: return "ldmib" + Disassemble_Cond(ir) + " " + Disassemble_Load_Store_Multiple(ir); break;
	case AInstructions::LDMDA: return "ldmda" + Disassemble_Cond(ir) + " " + Disassemble_Load_Store_Multiple(ir); break;
	case AInstructions::LDMDB: return "ldmdb" + Disassemble_Cond(ir) + " " + Disassemble_Load_Store_Multiple(ir); break;
	case AInstructions::LDMFD: return "ldmfd" + Disassemble_Cond(ir) + " " + Disassemble_Load_Store_Multiple(ir); break;
	case AInstructions::LDMFA: return "ldmfa" + Disassemble_Cond(ir) + " " + Disassemble_Load_Store_Multiple(ir); break;
	case AInstructions::LDMED: return "ldmed" + Disassemble_Cond(ir) + " " + Disassemble_Load_Store_Multiple(ir); break;
	case AInstructions::LDMEA: return "ldmea" + Disassemble_Cond(ir) + " " + Disassemble_Load_Store_Multiple(ir); break;

	case AInstructions::STMIA: return "stmia" + Disassemble_Cond(ir) + " " + Disassemble_Load_Store_Multiple(ir); break;
	case AInstructions::STMIB: return "stmib" + Disassemble_Cond(ir) + " " + Disassemble_Load_Store_Multiple(ir); break;
	case AInstructions::STMDA: return "stmda" + Disassemble_Cond(ir) + " " + Disassemble_Load_Store_Multiple(ir); break;
	case AInstructions::STMDB: return "stmdb" + Disassemble_Cond(ir) + " " + Disassemble_Load_Store_Multiple(ir); break;
	case AInstructions::STMFD: return "stmfd" + Disassemble_Cond(ir) + " " + Disassemble_Load_Store_Multiple(ir); break;
	case AInstructions::STMFA: return "stmfa" + Disassemble_Cond(ir) + " " + Disassemble_Load_Store_Multiple(ir); break;
	case AInstructions::STMED: return "stmed" + Disassemble_Cond(ir) + " " + Disassemble_Load_Store_Multiple(ir); break;
	case AInstructions::STMEA: return "stmea" + Disassemble_Cond(ir) + " " + Disassemble_Load_Store_Multiple(ir); break;

	//Semaphore
	case AInstructions::SWP: return "swp" + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Reg(ir.operand1) + ", [" + Disassemble_Reg(ir.operand3) + "]";  break;
	case AInstructions::SWPB: return "swpb" + Disassemble_Cond(ir) + " " + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Reg(ir.operand1) + ", [" + Disassemble_Reg(ir.operand3) + "]";  break;

	//Exception Generating
	case AInstructions::SWI: return "swi" + Disassemble_Cond(ir) + " #" + std::to_string(ir.operand1);

	//Coprocessor
	case AInstructions::CDP: return "cdp" + Disassemble_Cond(ir) + " p" + std::to_string(ir.operand3 & 0xF) + ", " + std::to_string(ir.operand4) + ", " + Disassemble_Reg((ir.operand3 >> 4) & 0xF) + ", c" + std::to_string((ir.operand3 >> 8) & 0xF) + ", c" + std::to_string(ir.operand1) + ", " + std::to_string(ir.operand2); break;
	case AInstructions::MCR: return "mcr" + Disassemble_Cond(ir) + " p" + std::to_string(ir.operand3 & 0xF) + ", " + std::to_string(ir.operand4) + ", " + Disassemble_Reg((ir.operand3 >> 4) & 0xF) + ", c" + std::to_string((ir.operand3 >> 8) & 0xF) + ", c" + std::to_string(ir.operand1) + ", " + std::to_string(ir.operand2); break;
	case AInstructions::MRC: return "mrc" + Disassemble_Cond(ir) + " p" + std::to_string(ir.operand3 & 0xF) + ", " + std::to_string(ir.operand4) + ", " + Disassemble_Reg((ir.operand3 >> 4) & 0xF) + ", c" + std::to_string((ir.operand3 >> 8) & 0xF) + ", c" + std::to_string(ir.operand1) + ", " + std::to_string(ir.operand2); break;

	case AInstructions::STC: return "stc" + Disassemble_Cond(ir); break;
	case AInstructions::LDC: return "ldc" + Disassemble_Cond(ir); break;
	}
	return std::string();
}

std::string Disassembler::Disassemble_Reg(u32 reg) {
	switch (reg) {
	case Regs::SP: return "sp";
	case Regs::LR: return "lr";
	case Regs::PC: return "pc";
	default: return "r" + std::to_string(reg);
	}
}

std::string Disassembler::Disassemble_Cond(IR_ARM & ir) {
	switch (ir.cond) {
	case Conditions::EQ: return "eq";
	case Conditions::NE: return "ne";
	case Conditions::CS_HS: return "cs";
	case Conditions::CC_LO: return "cc";
	case Conditions::MI: return "mi";
	case Conditions::PL: return "pl";
	case Conditions::VS: return "vs";
	case Conditions::VC: return "vc";
	case Conditions::HI: return "hi";
	case Conditions::LS: return "ls";
	case Conditions::GE: return "ge";
	case Conditions::LT: return "lt";
	case Conditions::GT: return "gt";
	case Conditions::LE: return "le";
	case Conditions::AL: return "";
	case Conditions::NV: return "nv";
	}
	throw std::string("invalid condition");
}

std::string Disassembler::Disassemble_Shifter_Operand(Shifter_op& so, bool negatif) {
	std::string negation = negatif ? "-" : "";
	switch (so.type) {
	case Immediate: return "#" + negation + std::to_string(so.operand1);
	case Register: return negation + Disassemble_Reg(so.operand1);
	case LSL_imm: return Disassemble_Reg(so.operand1) + ", lsl #" + negation + std::to_string(so.operand2);
	case LSL_reg: return Disassemble_Reg(so.operand1) + ", lsl " + Disassemble_Reg(so.operand2);
	case LSR_imm: return Disassemble_Reg(so.operand1) + ", lsr #" + negation + std::to_string(so.operand2);
	case LSR_reg: return Disassemble_Reg(so.operand1) + ", lsr " + Disassemble_Reg(so.operand2);
	case ASR_imm: return Disassemble_Reg(so.operand1) + ", asr #" + negation + std::to_string(so.operand2);
	case ASR_reg: return Disassemble_Reg(so.operand1) + ", asr " + Disassemble_Reg(so.operand2);
	case ROR_imm: return Disassemble_Reg(so.operand1) + ", ror #" + negation + std::to_string(so.operand2);
	case ROR_reg: return Disassemble_Reg(so.operand1) + ", ror " + Disassemble_Reg(so.operand2);
	case RRX: return Disassemble_Reg(so.operand1) + ", rrx";
	}
	throw std::string("invalid shifter operand");
}

std::string Disassembler::Disassemble_Shifter_Operand(Shifter_op& so) {
	return Disassemble_Shifter_Operand(so, false);
}

std::string Disassembler::Disassemble_Branch_Offset(u32 operand) {
	s32 offset = SignExtend<s32>(operand << 2, 26) + 8;
	std::string sign = offset >= 0 ? "+" : "-";
	return sign + "#" + std::to_string(abs(offset));
}

std::string Disassembler::Disassemble_PSR(u32 R) {
	return R == 1 ? "SPSR" : "CPSR";
}

std::string Disassembler::Disassemble_Fields(u32 fields) {
	std::string r = "";
	r += getBit(fields, 3) == 1 ? "f" : "";
	r += getBit(fields, 2) == 1 ? "s" : "";
	r += getBit(fields, 1) == 1 ? "x" : "";
	r += getBit(fields, 0) == 1 ? "c" : "";
	return r;
}

std::string Disassembler::Disassemble_LS_Shifter_Operand(IR_ARM& ir) {
	bool P = (ir.operand3 & 0b1000) >> 3 == 1;
	bool U = (ir.operand3 & 0b0100) >> 2== 1;
	bool W = (ir.operand3 & 0b0001) == 1;

	if (!P) {
		return "[" + Disassemble_Reg(ir.operand2) + "], " + Disassemble_Shifter_Operand(ir.shifter_operand, !U);
	}
	else if (P && W) {
		return "[" + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Shifter_Operand(ir.shifter_operand, !U) + "]!";
	}
	else {
		return "[" + Disassemble_Reg(ir.operand2) + ", " + Disassemble_Shifter_Operand(ir.shifter_operand, !U) + "]";
	}
}

std::string Disassembler::Disassemble_Load_Store_Multiple(IR_ARM& ir) {
	bool I = (ir.operand3 & 0b0010) >> 1 == 1;
	bool W = (ir.operand3 & 0b0001) == 1;

	return Disassemble_Reg(ir.operand2) + (W ? "!" : "") + ", " + Disassemble_Reg_List(ir.operand1) + (I ? "^" : "");
}

std::string Disassembler::Disassemble_Reg_List(u32 list) {
	std::string result = "{";
	bool current = false;
	unsigned start = 0;
	unsigned end = 0;
	
	for (int i = 0; i < 16; i++) {
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
	return result.substr(0, result.length()-1) + "}";
}