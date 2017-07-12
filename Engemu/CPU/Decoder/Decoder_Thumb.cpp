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
	
	if (((instr >> 10) & 0b11'1111) == 0b010000) {
		Decode_Data_Processing_Register(ir, instr);
		return;
	}

	if (((instr >> 8) & 0b1111'1111) == 0b01000111) {
		Decode_Branch_With_Exchange(ir, instr);
		return;
	}

	if (((instr >> 10) & 0b11'1111) == 0b010001) {
		Decode_Special_Data_Processing(ir, instr);
		return;
	}

	if (((instr >> 11) & 0b1'1111) == 0b01001) {
		Decode_Load_PC(ir, instr);
		return;
	}

	switch ((instr >> 12) & 0b1111) {
	case 0b0101: Decode_Load_Store_Reg_offset(ir, instr); return;
	case 0b0110:
	case 0b0111: 
	case 0b1000: Decoder_Load_Store_W_B_H_imm(ir, instr); return;
	case 0b1001: Decode_Load_Store_SP(ir, instr); return;
	case 0b1010: Decode_Add_To_PC_SP(ir, instr); return;
	case 0b1011: 
		switch ((instr >> 8) & 0xF) {
		case 0b0000: Decode_Adjust_SP(ir, instr); break;
		case 0b1110: throw std::string("Could not decode Thumb instruction"); break;
		default: Decode_Push_Pop(ir, instr); break;
		}
		return;
	case 0b1100: Decode_Load_Store_Multiple(ir, instr); return;
	case 0b1101: 
		switch ((instr >> 8) & 0xF) {
		case 0b1110: throw std::string("Undefined instruction"); break;
		case 0b1111: Decode_Exception_Generating(ir, instr); break;
		default: Decode_Conditional_Branch(ir, instr); break;
		}
		return;
	}

	throw std::string("Could not decode Thumb instruction");
}

void Decoder::Decode_Conditional_Branch(IR_Thumb& ir, u16 instr) {
	ir.type = TInstructionType::Branch;
	ir.instr = TInstructions::B_cond;
	ir.cond = static_cast<Conditions>((instr >> 8) & 0xF);
	ir.operand1 = instr & 0xFF; // signed immed
}

void Decoder::Decode_Unconditionnal_Branch(IR_Thumb& ir, u16 instr) {
	ir.type = TInstructionType::Branch;
	unsigned H = (instr >> 11) & 0b11;

	switch (H) {
	case 0b00: ir.instr = TInstructions::B_imm; break;
	case 0b10: ir.instr = TInstructions::BL_high; break;
	case 0b11: ir.instr = TInstructions::BL; break;
	}

	ir.operand1 = instr & 0x7FF; //immed
	ir.operand2 = H;
}

void Decoder::Decode_Branch_With_Exchange(IR_Thumb& ir, u16 instr) {
	ir.type = TInstructionType::Branch;

	switch (getBit(instr, 7)) {
	case 0: ir.instr = TInstructions::BX; break;
	}

	ir.operand1 = (instr >> 3) & 0xF; //H2|Rm
}

void Decoder::Decode_Add_Sub_reg_imm(IR_Thumb& ir, u16 instr) {
	ir.type = getBit(instr, 10) == 1 ? TInstructionType::Data_Processing_2 : TInstructionType::Data_Processing_1;
	
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
	ir.type = TInstructionType::Data_Processing_4;

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
	ir.type = TInstructionType::Data_Processing_3;

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
	ir.type = TInstructionType::Data_Processing_5;

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
	ir.type = TInstructionType::Data_Processing_6;
	
	switch (getBit(instr, 11)) {
	case 0: ir.instr = TInstructions::ADD_imm_pc; break;
	case 1: ir.instr = TInstructions::ADD_imm_sp; break;
	}

	ir.operand1 = instr & 0xFF; //immed
	ir.operand2 = (instr >> 8) & 0b111; //Rd
}

void Decoder::Decode_Adjust_SP(IR_Thumb& ir, u16 instr) {
	ir.type = TInstructionType::Data_Processing_7;

	switch (getBit(instr, 7)) {
	case 0: ir.instr = TInstructions::ADD_inc_sp; break;
	case 1: ir.instr = TInstructions::SUB_dec_sp; break;
	}

	ir.operand1 = instr & 0b111'1111;
}

void Decoder::Decode_Special_Data_Processing(IR_Thumb& ir, u16 instr) {
	ir.type = TInstructionType::Data_Processing_8;

	switch ((instr >> 8) & 0b11) {
	case 0b00: ir.instr = TInstructions::ADD_hig_reg; break;
	case 0b01: ir.instr = TInstructions::CMP_hig_reg; break;
	case 0b10: ir.instr = TInstructions::MOV_hig_reg; break;
	}

	ir.operand1 = (getBit(instr, 7) << 3) | (instr & 0b111); // H1|Rd/Rn
	ir.operand2 = (instr >> 3) & 0xF; // H2|Rm
}

void Decoder::Decoder_Load_Store_W_B_H_imm(IR_Thumb& ir, u16 instr) {
	ir.type = TInstructionType::Load_Store;

	switch ((instr >> 11) & 0b1'1111) {
	case 0b01100: ir.instr = TInstructions::STR_imm; break;
	case 0b01101: ir.instr = TInstructions::LDR_imm; break;
	case 0b01110: ir.instr = TInstructions::STRB_imm; break;
	case 0b01111: ir.instr = TInstructions::LDRB_imm; break;
	case 0b10000: ir.instr = TInstructions::STRH_imm; break;
	case 0b10001: ir.instr = TInstructions::LDRH_imm; break;
	}

	ir.operand1 = instr & 0b111; //Rd
	ir.operand2 = (instr >> 3) & 0b111; //Rn
	ir.operand3 = (instr >> 6) & 0b11111; //immed
}

void Decoder::Decode_Load_Store_Reg_offset(IR_Thumb& ir, u16 instr) {
	ir.type = TInstructionType::Load_Store;

	switch ((instr >> 9) & 0b111) {
	case 0b000: ir.instr = TInstructions::STR_reg; break;
	case 0b001: ir.instr = TInstructions::STRH_reg; break;
	case 0b010: ir.instr = TInstructions::STRB_reg; break;
	case 0b011: ir.instr = TInstructions::LDRSB; break;
	case 0b100: ir.instr = TInstructions::LDR_reg; break;
	case 0b101: ir.instr = TInstructions::LDRH_reg; break;
	case 0b110: ir.instr = TInstructions::LDRB_reg; break;
	case 0b111: ir.instr = TInstructions::LDRSH; break;
	}

	ir.operand1 = instr & 0b111; //Rd
	ir.operand2 = (instr >> 3) & 0b111; //Rn
	ir.operand3 = (instr >> 6) & 0b111; //Rm
}

void Decoder::Decode_Load_PC(IR_Thumb& ir, u16 instr) {
	ir.type = TInstructionType::Load_Store;
	ir.instr = TInstructions::LDR_pc;
	ir.operand1 = instr & 0xFF; //immed
	ir.operand2 = (instr >> 8) & 0b111; //Rd
}

void Decoder::Decode_Load_Store_SP(IR_Thumb& ir, u16 instr) {
	ir.type = TInstructionType::Load_Store;
	ir.instr = getBit(instr, 11) == 1 ? TInstructions::LDR_sp : TInstructions::STR_sp;
	ir.operand1 = instr & 0xFF; //immed
	ir.operand2 = (instr >> 8) & 0b111; //Rd
}

void Decoder::Decode_Load_Store_Multiple(IR_Thumb& ir, u16 instr) {
	ir.type = TInstructionType::Load_Store_Multiple;
	ir.instr = getBit(instr, 11) == 1 ? TInstructions::LDMIA : TInstructions::STMIA;
	ir.operand1 = instr & 0xFF; //reg_list
	ir.operand2 = (instr >> 8) & 0b111; //Rn
}

void Decoder::Decode_Push_Pop(IR_Thumb& ir, u16 instr) {
	ir.type = TInstructionType::Load_Store_Multiple;
	ir.instr = getBit(instr, 11) == 1 ? TInstructions::POP : TInstructions::PUSH;
	ir.operand1 = instr & 0xFF; //reg_list
	ir.operand2 = getBit(instr, 8); //R
}

void Decoder::Decode_Exception_Generating(IR_Thumb& ir, u16 instr) {
	ir.type = TInstructionType::Exception_Generating;
	ir.instr = TInstructions::SWI;
	ir.operand1 = instr & 0xFF;
}