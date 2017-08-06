#include "CPU.h"
#include "Utils.h"
#include "Decoder/Decoder.h"
#include "../Symbols/SymbolsManager.h"

void CPU::Execute(IR_Thumb& ir) {
	switch (ir.type) {
	case TInstructionType::Data_Processing_1: Data_Processing_1_2(ir); break;
	case TInstructionType::Data_Processing_2: Data_Processing_1_2(ir); break;
	case TInstructionType::Data_Processing_3: Data_Processing_3(ir); break;
	case TInstructionType::Data_Processing_4: Data_Processing_4(ir); break;
	case TInstructionType::Data_Processing_5: Data_Processing_5(ir); break;
	case TInstructionType::Data_Processing_6: Data_Processing_6_7(ir); break;
	case TInstructionType::Data_Processing_7: Data_Processing_6_7(ir); break;
	case TInstructionType::Data_Processing_8: Data_Processing_8(ir); break;
	case TInstructionType::Branch: Branch(ir); break;
	case TInstructionType::Load_Store: Load_Store(ir); break;
	case TInstructionType::Load_Store_Multiple: Load_Store_Multiple(ir); break;
	case TInstructionType::Exception_Generating: throw std::string("Unimplemented opcode"); break;
	}
}

void CPU::Load_Store_Multiple(IR_Thumb& ir) {
	u16& reg_list = ir.operand1;
	u16& Rn = ir.operand2;
	u16& R = ir.operand2;

	unsigned number_regs_modified = 0;

	for (int i = 0; i < 8; i++) {
		if (getBit(reg_list, i) == 1) {
			number_regs_modified++;
		}
	}

	switch (ir.instr) {

	case TInstructions::LDMIA: {
		u32 start_address = gprs[Rn];
		u32 end_address = gprs[Rn] + (number_regs_modified * 4) - 4;
		u32 address = start_address;
		for (int i = 0; i < 8; i++) {
			if (getBit(reg_list, i) == 1) {
				gprs[i] = mem.read32(address);
				address = address + 4;
			}
		}
		if (!(end_address == address - 4)) {
			throw std::string("Something went wrong in Load multiple");
		}
		gprs[Rn] = gprs[Rn] + (number_regs_modified * 4);
		break;
	}

	case TInstructions::STMIA: {
		u32 start_address = gprs[Rn];
		u32 end_address = gprs[Rn] + (number_regs_modified * 4) - 4;
		u32 address = start_address;
		for (int i = 0; i < 8; i++) {
			if (getBit(reg_list, i) == 1) {
				mem.write32(address, gprs[i]);
				address = address + 4;
			}
		}
		if (!(end_address == address - 4)) {
			throw std::string("Something went wrong in Load multiple");
		}
		gprs[Rn] = gprs[Rn] + (number_regs_modified * 4);
		break;
	}
	case TInstructions::POP: {
		u32 start_address = gprs[Regs::SP];
		u32 end_address = gprs[Regs::SP] + 4 * (R + number_regs_modified);
		u32 address = start_address;

		for (int i = 0; i < 8; i++) {
			if (getBit(reg_list, i) == 1) {
				gprs[i] = mem.read32(address);
				address = address + 4;
			}
		}

		if (R == 1) {
			gprs[Regs::PC] = mem.read32(address) & 0xFFFFFFFE;
			address = address + 4;
		}

		if (!(end_address == address)) {
			throw std::string("Something went wrong in Load multiple");
		}
		gprs[Regs::SP] = end_address;
		break;
	}

	case TInstructions::PUSH: {
		u32 start_address = gprs[Regs::SP] - 4 * (R + number_regs_modified);
		u32 end_address = gprs[Regs::SP] - 4;
		u32 address = start_address;
		for (int i = 0; i < 8; i++) {
			if (getBit(reg_list, i) == 1) {
				mem.write32(address, gprs[i]);
				address = address + 4;
			}
		}

		if (R == 1) {
			mem.write32(address, gprs[Regs::LR]);
			address = address + 4;
		}
		if (!(end_address == address - 4)) {
			throw std::string("Something went wrong in Load multiple");
		}
		gprs[Regs::SP] = gprs[Regs::SP] - 4 * (R + number_regs_modified);
		break;
	}
	}
}

void CPU::Load_Store(IR_Thumb& ir) {
	u16& Rd = ir.operand1;
	u16& Rn = ir.operand2;
	u16& immed = ir.operand3;
	u16& Rm = ir.operand3;

	switch (ir.instr) {
	case TInstructions::LDR_imm: {
		u32 address = gprs[Rn] + (immed * 4);
		if ((address & 0b11) == 0b00) gprs[Rd] = mem.read32(address);
		else throw std::string("unpredictable load of unaligned address");
		break;
	}
	case TInstructions::LDRB_imm: {
		u32 address = gprs[Rn] + immed;
		gprs[Rd] = mem.read8(address);
		break;
	}
	case TInstructions::LDRH_imm: {
		u32 address = gprs[Rn] + (immed * 2);
		if ((address & 0b1) == 0b0) gprs[Rd] = mem.read16(address);
		else throw std::string("unpredictable load of unaligned address");
		break;
	}
	case TInstructions::STR_imm: {
		u32 address = gprs[Rn] + (immed * 4);
		if ((address & 0b11) == 0b00) mem.write32(address, gprs[Rd]);
		else throw std::string("unpredictable load of unaligned address");
		break;
	}
	case TInstructions::STRB_imm: {
		u32 address = gprs[Rn] + immed;
		mem.write8(address, gprs[Rd]);
		break;
	}
	case TInstructions::STRH_imm: {
		u32 address = gprs[Rn] + (immed * 2);
		if ((address & 0b1) == 0b0) mem.write16(address, gprs[Rd]);
		else throw std::string("unpredictable load of unaligned address");
		break;
	}
	
	case TInstructions::LDR_reg: {
		u32 address = gprs[Rn] + gprs[Rm];
		if ((address & 0b11) == 0b00) gprs[Rd] = mem.read32(address);
		else throw std::string("unpredictable load of unaligned address");
		break;
	}
	case TInstructions::LDRB_reg: {
		u32 address = gprs[Rn] + gprs[Rm];
		gprs[Rd] = mem.read8(address);
		break;
	}
	case TInstructions::LDRH_reg: {
		u32 address = gprs[Rn] + gprs[Rm];
		if ((address & 0b1) == 0b0) gprs[Rd] = mem.read16(address);
		else throw std::string("unpredictable load of unaligned address");
		break;
	}

	case TInstructions::STR_reg: {
		u32 address = gprs[Rn] + gprs[Rm];
		if ((address & 0b11) == 0b00) mem.write32(address, gprs[Rd]);
		else throw std::string("unpredictable load of unaligned address");
		break;
	}
	case TInstructions::STRB_reg: {
		u32 address = gprs[Rn] + gprs[Rm];
		mem.write8(address, gprs[Rd]);
		break;
	}
	case TInstructions::STRH_reg: {
		u32 address = gprs[Rn] + gprs[Rm];
		if ((address & 0b1) == 0b0) mem.write16(address, gprs[Rd]);
		else throw std::string("unpredictable load of unaligned address");
		break;
	}
	
	case TInstructions::LDRSB: {
		u32 address = gprs[Rn] + gprs[Rm];
		gprs[Rd] = SignExtend<s32>(mem.read8(address), 8);
		break; 
	}
	case TInstructions::LDRSH: {
		u32 address = gprs[Rn] + gprs[Rm];
		if ((address & 0b1) == 0b0) gprs[Rd] = SignExtend<s32>(mem.read16(address), 16);
		else throw std::string("unpredictable load of unaligned address");
		break;
	}

	//from here the Rd, Rn, ... don't correspond anymore

	case TInstructions::LDR_pc: {
		u32 address = (gprs[Regs::PC] & 0xFFFFFFFC) + (ir.operand1 * 4);
		gprs[ir.operand2] = mem.read32(address);
		break;
	}

	case TInstructions::LDR_sp: {
		u32 address = gprs[Regs::SP] + (ir.operand1 * 4);
		if ((address & 0b11) == 0b00) gprs[ir.operand2] = mem.read32(address);
		else throw std::string("unpredictable load of unaligned address");
		break;
	}
	case TInstructions::STR_sp: {
		u32 address = gprs[Regs::SP] + (ir.operand1 * 4);
		if ((address & 0b11) == 0b00) mem.write32(address, gprs[ir.operand2]);
		else throw std::string("unpredictable load of unaligned address");
		break;
	}
	}
}

void CPU::Branch(IR_Thumb& ir) {

	switch (ir.instr) {
	case TInstructions::B_cond: gprs[Regs::PC] += SignExtend<s32>(ir.operand1 << 1, 9);  break;
	case TInstructions::B_imm:  gprs[Regs::PC] += SignExtend<s32>(ir.operand1 << 1, 12); break;
	case TInstructions::BL_high: gprs[Regs::LR] = gprs[Regs::PC] + (SignExtend<s32>(ir.operand1, 11) << 12);  break;
	case TInstructions::BL: {
		//TODO: Check that it works
		u32 next_instruction = (gprs[Regs::PC] - 2) | 1;
		gprs[Regs::PC] = gprs[Regs::LR] + (ir.operand1 << 1);
		gprs[Regs::LR] = next_instruction;
		call_stack.push_back(Symbols::getFunctionNameOrElse(gprs[Regs::PC]));
		break;
	}
	case TInstructions::BX:
		cpsr.flag_T = !!(gprs[ir.operand1] & 0b1);
		gprs[Regs::PC] = gprs[ir.operand1] & 0xFFFFFFFE;
		if (ir.operand1 == Regs::LR && !call_stack.empty()) {
			call_stack.pop_back();
		}
		else if (!call_stack.empty()) {
			//replace the last one with the new
			//Useful for import stubs
			call_stack.pop_back();
			call_stack.push_back(Symbols::getFunctionNameOrElse(gprs[Regs::PC]));
		}
		break;
	}
}

void CPU::Data_Processing_1_2(IR_Thumb& ir) {
	u16& Rd = ir.operand1;
	u16& Rn = ir.operand2;
	u16& Rm = ir.operand3;
	u16& immed = ir.operand3;

	auto fun_Rd_31 = [&]()->bool {return !!getBit(gprs[Rd], 31); };
	auto fun_Rd_0 = [&]()->bool {return gprs[Rd] == 0; };

	switch (ir.instr) {
	case TInstructions::ADD_reg: DP_Instr1(Rd, gprs[Rn] + gprs[Rm], fun_Rd_31, fun_Rd_0, [&]()->bool {return CarryFrom(gprs[Rn], gprs[Rm]); }, [&]()->bool {return OverflowFromAdd(gprs[Rn], gprs[Rm]); }); break;
	case TInstructions::SUB_reg: DP_Instr1(Rd, gprs[Rn] - gprs[Rm], fun_Rd_31, fun_Rd_0, [&]()->bool {return !BorrowFromSub(gprs[Rn], gprs[Rm]); }, [&]()->bool {return OverflowFromSub(gprs[Rn], gprs[Rm]); }); break;
	case TInstructions::ADD_imm: DP_Instr1(Rd, gprs[Rn] + immed, fun_Rd_31, fun_Rd_0, [&]()->bool {return CarryFrom(gprs[Rn], immed); }, [&]()->bool {return OverflowFromAdd(gprs[Rn], immed); }); break;
	case TInstructions::SUB_imm: DP_Instr1(Rd, gprs[Rn] - immed, fun_Rd_31, fun_Rd_0, [&]()->bool {return !BorrowFromSub(gprs[Rn], immed); }, [&]()->bool {return OverflowFromSub(gprs[Rn], immed); }); break;
	}
}

void CPU::Data_Processing_3(IR_Thumb& ir) {
	u16& immed = ir.operand1;
	u16& Rd = ir.operand2;

	auto fun_Rd_31 = [&]()->bool {return !!getBit(gprs[Rd], 31); };
	auto fun_Rd_0 = [&]()->bool {return gprs[Rd] == 0; };

	switch (ir.instr) {
	case TInstructions::ADD_lar_imm: DP_Instr1(Rd, immed + gprs[Rd], fun_Rd_31, fun_Rd_0, [&]()->bool {return CarryFrom(immed, gprs[Rd]); }, [&]()->bool {return OverflowFromAdd(immed, gprs[Rd]); }); break;
	case TInstructions::SUB_lar_imm: DP_Instr1(Rd, gprs[Rd] - immed, fun_Rd_31, fun_Rd_0, [&]()->bool {return !BorrowFromSub(gprs[Rd], immed); }, [&]()->bool {return OverflowFromSub(gprs[Rd], immed); }); break;
	case TInstructions::MOV_imm: DP_Instr1(Rd, immed, fun_Rd_31, fun_Rd_0, fun_C, fun_V); break;
	case TInstructions::CMP_imm: DP_Instr2(gprs[Rd] - immed, fun_r_31, fun_r_0, [&](u32 r)->bool {return !BorrowFromSub(gprs[Rd], immed); }, [&](u32 r)->bool {return OverflowFromSub(gprs[Rd], immed); }); break;
	}
}

void CPU::Data_Processing_4(IR_Thumb& ir) {
	u16& Rd = ir.operand1;
	u16& Rm = ir.operand2;
	u16& immed = ir.operand3;

	bool flag_C = cpsr.flag_C;
	u32 vRm = gprs[Rm];

	auto fun_Rd_31 = [&]()->bool {return !!getBit(gprs[Rd], 31); };
	auto fun_Rd_0 = [&]()->bool {return gprs[Rd] == 0; };

	//TODO: Is C flag using new values or old ? seems like it should be the old
	switch (ir.instr) {
	case TInstructions::LSL_imm: DP_Instr1(Rd, gprs[Rm] << immed, fun_Rd_31, fun_Rd_0, [&]()->bool {return immed == 0 ? flag_C : !!getBit(vRm, 32 - immed); }, fun_V); break;
	case TInstructions::LSR_imm: DP_Instr1(Rd, immed == 0 ? 0 : gprs[Rm] >> immed, fun_Rd_31, fun_Rd_0, [&]()->bool {return immed == 0 ? !!getBit(vRm, 31) : !!getBit(vRm, immed - 1); }, fun_V); break;
	case TInstructions::ASR_imm: DP_Instr1(Rd, immed == 0 ? (getBit(gprs[Rm], 31) == 0 ? 0 : 0xFFFFFFFF): s32(gprs[Rm]) >> immed, fun_Rd_31, fun_Rd_0, [&]()->bool {return immed == 0 ? !!getBit(vRm, 31) : !!getBit(vRm, immed - 1); }, fun_V); break;
	}
}

void CPU::Data_Processing_5(IR_Thumb& ir) {
	u16& Rd = ir.operand1;
	u16& Rm = ir.operand2;
	u16& Rn = ir.operand1;
	u16& Rs = ir.operand2;

	auto fun_Rd_31 = [&]()->bool {return !!getBit(gprs[Rd], 31); };
	auto fun_Rd_0 = [&]()->bool {return gprs[Rd] == 0; };

	//TODO: Check that neg works correctly (doubt because unsigned substraction)
	//TODO: Check the borrow from add for CMM

	switch (ir.instr) {
	case TInstructions::AND: DP_Instr1(Rd, gprs[Rd] & gprs[Rm], fun_Rd_31, fun_Rd_0, fun_C, fun_V); break;
	case TInstructions::EOR: DP_Instr1(Rd, gprs[Rd] ^ gprs[Rm], fun_Rd_31, fun_Rd_0, fun_C, fun_V); break;
	case TInstructions::ORR: DP_Instr1(Rd, gprs[Rd] | gprs[Rm], fun_Rd_31, fun_Rd_0, fun_C, fun_V); break;
	case TInstructions::BIC: DP_Instr1(Rd, gprs[Rd] & ~gprs[Rm], fun_Rd_31, fun_Rd_0, fun_C, fun_V); break;
	case TInstructions::ADC: DP_Instr1(Rd, gprs[Rd] + gprs[Rm] + cpsr.flag_C, fun_Rd_31, fun_Rd_0, [&]()->bool {return CarryFrom(gprs[Rd], gprs[Rm], cpsr.flag_C); }, [&]()->bool {return OverflowFromAdd(gprs[Rd], gprs[Rm], cpsr.flag_C); }); break;
	case TInstructions::SBC: DP_Instr1(Rd, gprs[Rd] - gprs[Rm] - !cpsr.flag_C, fun_Rd_31, fun_Rd_0, [&]()->bool {return !BorrowFromSub(gprs[Rd], gprs[Rm], !cpsr.flag_C); }, [&]()->bool {return OverflowFromSub(gprs[Rd], gprs[Rm], !cpsr.flag_C); }); break;
	case TInstructions::MVN: DP_Instr1(Rd, ~gprs[Rm], fun_Rd_31, fun_Rd_0, fun_C, fun_V); break;
	case TInstructions::MUL: DP_Instr1(Rd, getLo(static_cast<u64>(Rd) * static_cast<u64>(Rm)), fun_Rd_31, fun_Rd_0, fun_C, fun_V); break;
	case TInstructions::NEG: DP_Instr1(Rd, 0 - gprs[Rm], fun_Rd_31, fun_Rd_0, [&]()->bool {return !BorrowFromSub(0, gprs[Rm]); }, [&]()->bool {return OverflowFromSub(0, gprs[Rm]); }); break;

	case TInstructions::TST:     DP_Instr2(gprs[Rn] & gprs[Rm], fun_r_31, fun_r_0, fun_r_C, fun_r_V); break;
	case TInstructions::CMP_reg: DP_Instr2(gprs[Rn] - gprs[Rm], fun_r_31, fun_r_0, [&](u32 r)->bool {return !BorrowFromSub(gprs[Rn], gprs[Rm]); }, [&](u32 r)->bool {return OverflowFromSub(gprs[Rn], gprs[Rm]); }); break;
	case TInstructions::CMN:     DP_Instr2(gprs[Rn] + gprs[Rm], fun_r_31, fun_r_0, [&](u32 r)->bool {return !BorrowFromAdd(gprs[Rn], gprs[Rm]); }, [&](u32 r)->bool {return OverflowFromAdd(gprs[Rn], gprs[Rm]); }); break;

	case TInstructions::LSL_reg: {
		unsigned vRs_7_0 = gprs[Rs] & 0xFF;
		if (vRs_7_0 > 0 && vRs_7_0 < 32) { cpsr.flag_C = !!getBit(gprs[Rd], 32 - vRs_7_0); gprs[Rd] = gprs[Rd] << vRs_7_0; }
		else if (vRs_7_0 == 32) { cpsr.flag_C = !!getBit(gprs[Rd], 0); gprs[Rd] = 0; }
		cpsr.flag_N = fun_Rd_31();
		cpsr.flag_Z = fun_Rd_0();
	}
	break;
	case TInstructions::LSR_reg: {
		unsigned vRs_7_0 = gprs[Rs] & 0xFF;
		if (vRs_7_0 > 0 && vRs_7_0 < 32) { cpsr.flag_C = !!getBit(gprs[Rd], vRs_7_0 - 1); gprs[Rd] = gprs[Rd] >> vRs_7_0; }
		else if (vRs_7_0 == 32) { cpsr.flag_C = fun_Rd_31(); gprs[Rd] = 0; }
		cpsr.flag_N = fun_Rd_31();
		cpsr.flag_Z = fun_Rd_0();
	}
	break;
	case TInstructions::ASR_reg: {
		unsigned vRs_7_0 = gprs[Rs] & 0xFF;
		if (vRs_7_0 > 0 && vRs_7_0 < 32) { cpsr.flag_C = !!getBit(gprs[Rd], vRs_7_0 - 1); gprs[Rd] = u32(s32(gprs[Rd]) >> vRs_7_0); }
		else if (vRs_7_0 == 32) { cpsr.flag_C = fun_Rd_31(); if (cpsr.flag_C) gprs[Rd] = 0xFFFFFFFF; else gprs[Rd] = 0; }
		cpsr.flag_N = fun_Rd_31();
		cpsr.flag_Z = fun_Rd_0();
	}
	break;
	case TInstructions::ROR: {
		unsigned vRs_7_0 = gprs[Rs] & 0xFF;
		unsigned vRs_4_0 = gprs[Rs] & 0b1'1111;
		if (vRs_7_0 == 0) {}
		else if (vRs_4_0 == 0) { cpsr.flag_C = fun_Rd_31(); }
		else if (vRs_4_0 == 0) { cpsr.flag_C = !!getBit(gprs[Rd], vRs_4_0 - 1); gprs[Rd] = ror32(gprs[Rd], vRs_4_0); }
		cpsr.flag_N = fun_Rd_31();
		cpsr.flag_Z = fun_Rd_0();
	}
	break;
	}
}

void CPU::Data_Processing_6_7(IR_Thumb& ir) {
	u16& immed = ir.operand1;
	u16& Rd = ir.operand2;

	//Is bit 1 of pc cleared ? (see stackoverflow about pc +  8/4)
	switch (ir.instr) {
	case TInstructions::ADD_imm_pc: gprs[Rd] = (gprs[Regs::PC] & 0xFFFFFFFC) + (immed << 2);  break;
	case TInstructions::ADD_imm_sp: gprs[Rd] = gprs[Regs::SP] + (immed << 2); break;
	case TInstructions::ADD_inc_sp: gprs[Regs::SP] = gprs[Regs::SP] + (immed << 2); break;
	case TInstructions::SUB_dec_sp: gprs[Regs::SP] = gprs[Regs::SP] - (immed << 2); break;
	}
}

void CPU::Data_Processing_8(IR_Thumb& ir) {
	u16& Rd = ir.operand1;
	u16& Rn = ir.operand1;
	u16& Rm = ir.operand2;

	auto fun_Rd_31 = [&]()->bool {return !!getBit(gprs[Rd], 31); };
	auto fun_Rd_0 = [&]()->bool {return gprs[Rd] == 0; };

	switch (ir.instr) {
	case TInstructions::ADD_hig_reg: gprs[Rd] = gprs[Rd] + gprs[Rm]; break;
	case TInstructions::CMP_hig_reg: case TInstructions::CMP_reg: DP_Instr2(gprs[Rn] - gprs[Rm], fun_r_31, fun_r_0, [&](u32 r)->bool {return !BorrowFromSub(gprs[Rn], gprs[Rm]); }, [&](u32 r)->bool {return OverflowFromSub(gprs[Rn], gprs[Rm]); }); break;
	case TInstructions::MOV_hig_reg: gprs[Rd] = gprs[Rm];  break;
	}
}