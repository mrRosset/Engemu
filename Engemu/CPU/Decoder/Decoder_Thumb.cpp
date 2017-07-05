#include <string>
#include "Decoder.h"
#include "../Utils.h"

void Decoder::Decode(IR_Thumb& ir, u16 instr){
	//TODO: find a better way to detect if there is a condition field.
	ir.cond = Conditions::AL;

	if (((instr >> 13) & 0b111) == 0b000) {
		if (((instr >> 11) & 0b11) == 0b11) {
			Decode_Add_Sub_reg_imm(ir, instr);
			return;
		}
		else {
			Decode_Shift_Imm(ir, instr);
			return;
		}
	}
	
	switch ((instr >> 13) & 0b111) {
	case 0b001: Decode_Add_Sub_Mov_Cmp_imm(ir, instr); return;
	case 0b111: Decode_Unconditionnal_Branch(ir, instr); return;
	}
	
	if (((instr >> 10) & 0b111111) == 0b010000) {
		Decode_Data_Processing_Register(ir, instr);
		return;
	}

	if (((instr >> 8) & 0b11111111) == 0b01000111) {
		Decode_Branch_With_Exchange(ir, instr);
		return;
	}

	if (((instr >> 10) & 0b111111) == 0b010001) {
		Decode_Special_Data_Processing(ir, instr);
		return;
	}

	switch ((instr >> 12) & 0b1111) {
	case 0b0101: return;
	case 0b0110:
	case 0b0111: return;
	case 0b1000: return;
	case 0b1001: return;
	case 0b1010: Decode_Add_To_PC_SP(ir, instr); return;
	case 0b1011: 
		switch ((instr >> 8) & 0xF) {
		case 0b0000: Decode_Adjust_SP(ir, instr); break;
		case 0b1110: break;
		default: break;
		}
		return;
	case 0b1100: return;
	case 0b1101: 
		unsigned bits11_8 = (instr >> 8) & 0xF;
		switch (bits11_8) {
		case 0b1110: throw std::string("Undefined instruction");
		case 0b1111: break; //Software interrupts
		default:
			ir.type = InstructionType::Branch;
			ir.instr = TInstructions::B_cond;
			ir.cond = static_cast<Conditions>(bits11_8);
			ir.operand1 = instr & 0xFF; // signed immed
		}
		return;
	}

}

void Decoder::Decode_Unconditionnal_Branch(IR_Thumb& ir, u16 instr) {
	ir.type = InstructionType::Branch;
	unsigned H = (instr >> 11) & 0b11;

	switch (H) {
	case 0b00: ir.instr = TInstructions::B_imm; break;
	case 0b10: ir.instr = TInstructions::BL_high; break;
	case 0b01: ir.instr = TInstructions::BLX_imm; break;
	case 0b11: ir.instr = TInstructions::BL; break;
	}

	ir.operand1 = instr & 0x7FF; //immed
	ir.operand2 = H;
}

void Decoder::Decode_Branch_With_Exchange(IR_Thumb& ir, u16 instr) {
	ir.type = InstructionType::Branch;

	switch (getBit(instr, 7)) {
	case 0: ir.instr = TInstructions::BX; break;
	case 1: ir.instr = TInstructions::BLX_reg; break;
	}

	ir.operand1 = (instr >> 3) & 0xF; //H2|Rm
}

void Decoder::Decode_Add_Sub_reg_imm(IR_Thumb& ir, u16 instr) {
	ir.type = InstructionType::Data_Processing;
	
	switch ((instr >> 9) & 0b11) {
	case 0b00: ir.instr = TInstructions::ADD_reg; break;
	case 0b01: ir.instr = TInstructions::SUB_reg; break;
	case 0b10: ir.instr = TInstructions::ADD_imm; break;
	case 0b11: ir.instr = TInstructions::SUB_imm; break;
	}

	ir.operand1 = instr & 0b111; //Rd
	ir.operand2 = (instr >> 3) & 0b111; //Rn
	ir.operand3 = (instr >> 6) & 0b111; //Rm or immed
}

void Decoder::Decode_Shift_Imm(IR_Thumb& ir, u16 instr) {
	ir.type = InstructionType::Data_Processing;

	switch ((instr >> 11) & 0b11) {
	case 0b00: ir.instr = TInstructions::LSL_imm; break;
	case 0b01: ir.instr = TInstructions::LSR_imm; break;
	case 0b10: ir.instr = TInstructions::ASR_imm; break;
	}

	ir.operand1 = instr & 0b111; //Rd
	ir.operand2 = (instr >> 3) & 0b111; //Rm
	ir.operand3 = (instr >> 6) & 0b11111; //shift imm
}

void Decoder::Decode_Add_Sub_Mov_Cmp_imm(IR_Thumb& ir, u16 instr) {
	ir.type = InstructionType::Data_Processing;

	switch ((instr >> 11) & 0b11) {
	case 0b00: ir.instr = TInstructions::MOV_imm; break;
	case 0b01: ir.instr = TInstructions::CMP_imm; break;
	case 0b10: ir.instr = TInstructions::ADD_lar_imm; break;
	case 0b11: ir.instr = TInstructions::SUB_lar_imm; break;
	}

	ir.operand1 = instr & 0xFF; //immed
	ir.operand2 = (instr >> 8) & 0b111; //Rd/Rn
}

void Decoder::Decode_Data_Processing_Register(IR_Thumb& ir, u16 instr) {
	ir.type = InstructionType::Data_Processing;

	switch ((instr >> 6) & 0xF) {
	case 0b0000: ir.instr = TInstructions::AND; break;
	case 0b0001: ir.instr = TInstructions::EOR; break;
	case 0b0010: ir.instr = TInstructions::LSL_reg; break;
	case 0b0011: ir.instr = TInstructions::LSR_reg; break;
	case 0b0100: ir.instr = TInstructions::ASR_reg; break;
	case 0b0101: ir.instr = TInstructions::ADC; break;
	case 0b0110: ir.instr = TInstructions::SBC; break;
	case 0b0111: ir.instr = TInstructions::ROR; break;
	case 0b1000: ir.instr = TInstructions::TST; break;
	case 0b1001: ir.instr = TInstructions::NEG; break;
	case 0b1010: ir.instr = TInstructions::CMP_reg; break;
	case 0b1011: ir.instr = TInstructions::CMN; break;
	case 0b1100: ir.instr = TInstructions::ORR; break;
	case 0b1101: ir.instr = TInstructions::MUL; break;
	case 0b1110: ir.instr = TInstructions::BIC; break;
	case 0b1111: ir.instr = TInstructions::MVN; break;
	}

	ir.operand1 = instr & 0b111; // Rd/Rn
	ir.operand2 = (instr >> 3) & 0b111; // Rm/Rs
}

void Decoder::Decode_Add_To_PC_SP(IR_Thumb& ir, u16 instr) {
	ir.type = InstructionType::Data_Processing;
	
	switch (getBit(instr, 11)) {
	case 0: ir.instr = TInstructions::ADD_imm_pc; break;
	case 1: ir.instr = TInstructions::ADD_imm_sp; break;
	}

	ir.operand1 = instr & 0xFF; //immed
	ir.operand2 = (instr >> 8) & 0b111; //Rd
}

void Decoder::Decode_Adjust_SP(IR_Thumb& ir, u16 instr) {
	ir.type = InstructionType::Data_Processing;

	switch (getBit(instr, 7)) {
	case 0: ir.instr = TInstructions::ADD_inc_sp; break;
	case 1: ir.instr = TInstructions::SUB_dec_sp; break;
	}

	ir.operand1 = instr & 0b111'1111;
}

void Decoder::Decode_Special_Data_Processing(IR_Thumb& ir, u16 instr) {
	ir.type = InstructionType::Data_Processing;

	switch ((instr >> 8) & 0b11) {
	case 0b00: ir.instr = TInstructions::ADD_hig_reg; break;
	case 0b01: ir.instr = TInstructions::CMP_hig_reg; break;
	case 0b10: ir.instr = TInstructions::MOV_hig_reg; break;
	}

	ir.operand1 = (getBit(instr, 7) << 3) | (instr & 0b111); // H1|Rd/Rn
	ir.operand2 = (instr >> 3) & 0xF; // H2|Rm
}