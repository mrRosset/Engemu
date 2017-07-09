#include <string>
#include "Decoder.h"
#include "../Utils.h"


void Decoder::Decode(IR_ARM & ir, u32 instr) {
	ir.cond = static_cast<Conditions>((instr >> 28) & 0b1111);
	
	if (ir.cond == Conditions::NV) {
		throw std::string("undefined or unpredictable instrutions are not emulated");
	}

	unsigned bits28_25 = (instr >> 25) & 0b111;
	switch (bits28_25) {
	case 0b000:  // Data processing + Miscellaneous
	{
		unsigned bit4 = (instr >> 4) & 0b1;
		unsigned bit7 = (instr >> 7) & 0b1;
		unsigned bit20 = (instr >> 20) & 0b1;
		unsigned bit22 = (instr >> 22) & 0b1;
		unsigned bit24_23 = (instr >> 23) & 0b11;

		if (bit24_23 == 0b10 && bit20 == 0 && (bit4 == 0 || (bit4 == 1 && bit7 == 0) )) {
			unsigned bit7_4 = (instr >> 4) & 0xF;
			switch (bit7_4) {
			case 0b0000: Decode_Status_Register(ir, instr); break;
			case 0b0001: if(bit22 == 0) Decode_Branch(ir, instr);
						 else throw "3-3 not done"; break;
			case 0b0011:
			case 0b0101:
			case 0b0111:
			//case 0b1xy0:
			default:
				throw "3-3 not done";
			}
		}
		else if (bit7 == 1 && bit4 == 1) {
			//3-2
			unsigned bit7_4 = (instr >> 4) & 0xF;
			if (bit7_4 == 0b1001) { //should take care of SWP/SWPB
				if (getBit(instr, 24) == 0) {
					Decode_Multiply(ir, instr);
				} else {
					Decode_Semaphore(ir, instr);
				}
			}
			else {
				Decode_Load_Store_H_SB(ir, instr);
			}
		}
		else {
			Decode_Data_Processing(ir, instr);
		}
		break;
	}
	case 0b001:; // Data processing immediate + Move immediate to status register
	{
		unsigned bit21_20 = (instr >> 20) & 0b11;
		unsigned bit24_23 = (instr >> 23) & 0b11;
		if (bit24_23 == 0b10 && bit21_20 == 0b00) {
			throw "Undefined instruction are not emulated";
		}
		else if (bit24_23 == 0b10 && bit21_20 == 0b10) {
			Decode_Status_Register(ir, instr);
		}
		else {
			Decode_Data_Processing(ir, instr);
		}

		break;
	}
	case 0b010: Decode_Load_Store_W_UB(ir, instr); break; // Load / store immediate offset
	case 0b011: Decode_Load_Store_W_UB(ir, instr); break; // Load / store register offset
	case 0b100: Decode_Load_Store_Multiple(ir, instr); break; // Load/store multiple
	case 0b101: Decode_Branch(ir, instr); break; // Branch and branch with link
	case 0b110: throw std::string("Unimplemented opcode"); break; // Coprocessor load/store and double register transfers[<- does it exist without the DSP extension ?
	case 0b111: if (getBit(instr, 24) == 1) Decode_Exception_Generating(ir, instr);
		else throw std::string("Unimplemented opcode"); // Coprocessor
		break; 
	default: throw std::string("Unimplemented opcode");
	}

}


void Decoder::Decode_Data_Processing(IR_ARM & ir, u32 instr) {
	ir.type = AInstructionType::Data_Processing;
	Decode_Shifter_operand(ir, instr);
	
	switch ((instr >> 21) & 0xF) {
	case 0b0000: ir.instr = AInstructions::AND; break;
	case 0b0001: ir.instr = AInstructions::EOR; break;
	case 0b0010: ir.instr = AInstructions::SUB; break;
	case 0b0011: ir.instr = AInstructions::RSB; break;
	case 0b0100: ir.instr = AInstructions::ADD; break;
	case 0b0101: ir.instr = AInstructions::ADC; break;
	case 0b0110: ir.instr = AInstructions::SBC; break;
	case 0b0111: ir.instr = AInstructions::RSC; break;
	case 0b1000: ir.instr = AInstructions::TST; break;
	case 0b1001: ir.instr = AInstructions::TEQ; break;
	case 0b1010: ir.instr = AInstructions::CMP; break;
	case 0b1011: ir.instr = AInstructions::CMN; break;
	case 0b1100: ir.instr = AInstructions::ORR; break;
	case 0b1101: ir.instr = AInstructions::MOV; break;
	case 0b1110: ir.instr = AInstructions::BIC; break;
	case 0b1111: ir.instr = AInstructions::MVN; break;
	}

	ir.s = ((instr >> 20) & 0b1) == 1; //S
	ir.operand1 = (instr >> 12) & 0xF; //Rd
	ir.operand2 = (instr >> 16) & 0xF; //Rn
}

void Decoder::Decode_Shifter_operand(IR_ARM& ir, u32 instr) {
	if (getBit(instr, 25)) {
		ir.shifter_operand = { Shifter_type::Immediate, ror32(instr & 0xFF, ((instr >> 8) & 0xF) * 2), (instr >> 8) & 0xF};
		return;
	}

	unsigned shift_imm = (instr >> 7) & 0b11111;
	unsigned Rs = (instr >> 8) & 0xF;
	unsigned Rm = instr & 0xF;

	switch ((instr >> 4) & 0b111) {
	case 0b000: if(shift_imm == 0) ir.shifter_operand = { Shifter_type::Register, Rm};
				else ir.shifter_operand = { Shifter_type::LSL_imm, Rm, shift_imm };
				break;
	case 0b001: ir.shifter_operand = { Shifter_type::LSL_reg, Rm, Rs }; break;
	case 0b010: ir.shifter_operand = { Shifter_type::LSR_imm, Rm, (shift_imm == 0 ? 32 : shift_imm) }; break;
	case 0b011: ir.shifter_operand = { Shifter_type::LSR_reg, Rm, Rs }; break;
	case 0b100: ir.shifter_operand = { Shifter_type::ASR_imm, Rm, (shift_imm == 0 ? 32 : shift_imm) }; break;
	case 0b101: ir.shifter_operand = { Shifter_type::ASR_reg, Rm, Rs }; break;
	case 0b110: if(shift_imm == 0) ir.shifter_operand = { Shifter_type::RRX, Rm};
				else ir.shifter_operand = { Shifter_type::ROR_imm, Rm, shift_imm };
				break;
	case 0b111: ir.shifter_operand = { Shifter_type::ROR_reg, Rm, Rs }; break;
	}
}

void Decoder::Decode_Branch(IR_ARM& ir, u32 instr) {
	ir.type = AInstructionType::Branch;
	switch ((instr >> 24) & 0xF) {
	case 0b1010: ir.instr = AInstructions::B; ir.operand1 = instr & 0xFFFFFF; break;
	case 0b1011: ir.instr = AInstructions::BL; ir.operand1 = instr & 0xFFFFFF; break;
	case 0b0001: ir.instr = AInstructions::BX; ir.operand1 = instr & 0xF; break;
	}
}

void Decoder::Decode_Multiply(IR_ARM& ir, u32 instr) {
	ir.type = AInstructionType::Multiply;

	switch ((instr >> 21) & 0xF) {
	case 0b0000: ir.instr = AInstructions::MUL; break;
	case 0b0001: ir.instr = AInstructions::MLA; break;
	case 0b0100: ir.instr = AInstructions::UMULL; break;
	case 0b0101: ir.instr = AInstructions::UMLAL; break;
	case 0b0110: ir.instr = AInstructions::SMULL; break;
	case 0b0111: ir.instr = AInstructions::SMLAL; break;
	}

	ir.s = ((instr >> 20) & 0b1) == 1; //S
	ir.operand1 = instr & 0xF; //Rm
	ir.operand2 = (instr >> 8) & 0xF; //Rs
	ir.operand3 = (instr >> 12) & 0xF; //Rn or RdLo
	ir.operand4 = (instr >> 16) & 0xF; //Rd or RdHi
}

void Decoder::Decode_Status_Register(IR_ARM& ir, u32 instr) {
	ir.type = AInstructionType::Status_Regsiter_Access;
	switch ((instr >> 20) & 0b11111011) {
	case 0b00010000: ir.instr = AInstructions::MRS; ir.operand2 = (instr >> 12) & 0xF; break;
	case 0b00110010: ir.instr = AInstructions::MSR; ir.operand2 = (instr >> 16) & 0xF; ir.shifter_operand = { Shifter_type::Immediate, ror32(instr & 0xFF, ((instr >> 8) & 0xF) * 2), (instr >> 8) & 0xF}; break;
	case 0b00010010: ir.instr = AInstructions::MSR; ir.operand2 = (instr >> 16) & 0xF; ir.shifter_operand = { Shifter_type::Register,  instr & 0xFF }; break;
	}

	ir.operand1 = (instr >> 22) & 0b1; //R
}

void Decoder::Decode_Load_Store_W_UB(IR_ARM& ir, u32 instr) {
	ir.type = AInstructionType::Load_Store;
	switch ((instr >> 20) & 0b10111) {
	case 0b00000: ir.instr = AInstructions::STR; break;
	case 0b00001: ir.instr = AInstructions::LDR; break;
	case 0b00010: ir.instr = AInstructions::STRT; break;
	case 0b00011: ir.instr = AInstructions::LDRT; break;
	case 0b00100: ir.instr = AInstructions::STRB; break;
	case 0b00101: ir.instr = AInstructions::LDRB; break;
	case 0b00110: ir.instr = AInstructions::STRBT; break;
	case 0b00111: ir.instr = AInstructions::LDRBT; break;
	case 0b10000: ir.instr = AInstructions::STR; break;
	case 0b10001: ir.instr = AInstructions::LDR; break;
	case 0b10010: ir.instr = AInstructions::STR; break;
	case 0b10011: ir.instr = AInstructions::LDR; break;
	case 0b10100: ir.instr = AInstructions::STRB; break;
	case 0b10101: ir.instr = AInstructions::LDRB; break;
	case 0b10110: ir.instr = AInstructions::STRB; break;
	case 0b10111: ir.instr = AInstructions::LDRB; break;
	}

	ir.operand1 = (instr >> 12) & 0xF; //Rd
	ir.operand2 = (instr >> 16) & 0xF; //Rn
	ir.operand3 = (instr >> 21) & 0xF; //PUBW

	if (getBit(instr, 25) == 0) {
		//second argument is usually rotate_imm don't have here and don't care since it's used for carry out
		ir.shifter_operand = { Shifter_type::Immediate, instr & 0xFFF, 0}; 
	}
	else {
		u32 shift = (instr >> 5) & 0b11;
		u32 shift_imm = (instr >> 7) & 0x1F;

		if (shift == 0 && shift_imm == 0) {
			ir.shifter_operand = { Shifter_type::Register, instr & 0xF };
		}
		else {
			switch (shift) {
			case 0b00: ir.shifter_operand = { Shifter_type::LSL_imm, instr & 0xF,  shift_imm }; break;
			case 0b01: ir.shifter_operand = { Shifter_type::LSR_imm, instr & 0xF,  shift_imm == 0 ? 32 : shift_imm }; break;
			case 0b10: ir.shifter_operand = { Shifter_type::ASR_imm, instr & 0xF,  shift_imm == 0 ? 32 : shift_imm }; break;
			case 0b11: ir.shifter_operand = { shift_imm == 0 ? Shifter_type::RRX : Shifter_type::ROR_imm, instr & 0xF,  shift_imm }; break;
			}
		}
	}
}


void Decoder::Decode_Load_Store_H_SB(IR_ARM& ir, u32 instr) {
	ir.type = AInstructionType::Load_Store;
	//LSH
	switch ((getBit(instr, 20) << 2) | ((instr >> 5) & 0b11)) {
	case 0b000: throw std::string("Not a valid str instruction should be SWP or multiply");;
	case 0b001: ir.instr = AInstructions::STRH; break;
	case 0b010: throw std::string("UNPREDICTABLE instructions are not supported");
	case 0b011: throw std::string("UNPREDICTABLE instructions are not supported");
	case 0b100: ir.instr = AInstructions::LDRSB; break;
	case 0b101: ir.instr = AInstructions::LDRH; break;
	case 0b110: ir.instr = AInstructions::LDRSB; break; //TODO: verify. Normaly S = 1 means signed, but byte is always signed
	case 0b111: ir.instr = AInstructions::LDRSH; break;
	}

	ir.operand1 = (instr >> 12) & 0xF; //Rd
	ir.operand2 = (instr >> 16) & 0xF; //Rn
	ir.operand3 = (instr >> 21) & 0xF; //PUIW

	//I
	switch (getBit(instr, 22)) {
	//second argument is usually rotate_imm don't have here and don't care since it's used for carry out
	case 1: ir.shifter_operand = { Shifter_type::Immediate, ((instr & 0xF00) >> 4 )| (instr & 0xF), 0 };  break;
	case 0: ir.shifter_operand = { Shifter_type::Register, instr & 0xF }; break;
	}

}

void Decoder::Decode_Load_Store_Multiple(IR_ARM& ir, u32 instr) {
	ir.type = AInstructionType::Load_Store_Multiple;

	ir.operand1 = instr & 0xFFFF; //Regs list
	ir.operand2 = (instr >> 16) & 0xF; //Rn
	ir.operand3 = (instr >> 21) & 0xF; //PUSW

	//The ARM is not very clear: When should alternate name be used ?
	//According to arm it's when the operation if on the stack.
	//So only when Rn == SP ? It would be hard to statically determine
	//If any other Register contains a pointer to the stack

	//PUL
	switch (((instr >> 22) & 0b110) | getBit(instr, 20)) {
	case 0b001: ir.instr = ir.operand2 == Regs::SP ? AInstructions::LDMFA : AInstructions::LDMDA; break;
	case 0b011: ir.instr = ir.operand2 == Regs::SP ? AInstructions::LDMFD : AInstructions::LDMIA; break;
	case 0b101: ir.instr = ir.operand2 == Regs::SP ? AInstructions::LDMEA : AInstructions::LDMDB; break;
	case 0b111: ir.instr = ir.operand2 == Regs::SP ? AInstructions::LDMED : AInstructions::LDMIB; break;
	case 0b000: ir.instr = ir.operand2 == Regs::SP ? AInstructions::STMED : AInstructions::STMDA; break;
	case 0b010: ir.instr = ir.operand2 == Regs::SP ? AInstructions::STMEA : AInstructions::STMIA; break;
	case 0b100: ir.instr = ir.operand2 == Regs::SP ? AInstructions::STMFD : AInstructions::STMDB; break;
	case 0b110: ir.instr = ir.operand2 == Regs::SP ? AInstructions::STMFA : AInstructions::STMIB; break;
	}

	if (ir.operand1 == 0) {
		throw std::string("Unpredictable instructions are not emulated");
	}

}

void Decoder::Decode_Semaphore(IR_ARM& ir, u32 instr) {
	ir.type = AInstructionType::Semaphore;
	
	if (getBit(instr, 22) == 0) ir.instr = AInstructions::SWP;
	else ir.instr = AInstructions::SWPB;

	ir.operand1 = instr & 0xF; //Rm
	ir.operand2 = (instr >> 12) & 0xF; //Rd
	ir.operand3 = (instr >> 16) & 0xF; //Rn
}

void Decoder::Decode_Exception_Generating(IR_ARM& ir, u32 instr) {
	ir.type = AInstructionType::Exception_Generating;
	ir.instr = AInstructions::SWI;
	ir.operand1 = instr & 0xFFFFFF;
}

void Decoder::Decode_Coprocessor(IR_ARM& ir, u32 instr) {
	ir.type = AInstructionType::Coprocessor;
	
	if (getBit(instr, 25) == 1) {
		if (getBit(instr, 4) == 0) {
			ir.operand4 = (instr >> 20) & 0xF; //opcode_1
			ir.instr = AInstructions::CDP;
		} else {
			ir.operand4 = (instr >> 21) & 0b111; //opcode_1
			if (getBit(instr, 20) == 0) {
				ir.instr = AInstructions::MCR;
			} else {
				ir.instr = AInstructions::MRC;
			}
		}
		ir.operand1 = instr & 0xF; //CRm
		ir.operand2 = (instr >> 5) & 0b111; //opcode_2
		ir.operand3 = (instr >> 8) & 0xFFF; //Crn | CRd | cp_num

	} else {
		if (getBit(instr, 20) == 0) {
			ir.instr = AInstructions::STC;
		} else {
			ir.instr = AInstructions::LDC;
		}
		ir.operand1 = (instr >> 8) & 0xF; //cp_num
		ir.operand2 = (instr >> 16) & 0xF; //Rn
		ir.operand3 = (instr >> 21) & 0xF; //PUNW
		ir.operand4 = (instr >> 12) & 0xF; //CRd
		ir.shifter_operand = { Shifter_type::Immediate, (instr & 0xFF) * 4, 0 }; 
	}
}

/*For decoding co-processor:
TODO: Read and take care if necessary
	"The coprocessor double register transfer instructions are described in Chapter A10 Enhanced DSP Extension."
*/