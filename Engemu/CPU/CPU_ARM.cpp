#include <stdlib.h>
#include "CPU.h"
#include "Utils.h"
#include "Decoder/Decoder.h"

void CPU::Execute(IR_ARM& ir) {
	switch (ir.type) {
	case AInstructionType::Data_Processing: Data_Processing(ir); break;
	case AInstructionType::Branch: Branch(ir); break;
	case AInstructionType::Multiply: Multiply(ir); break;
	case AInstructionType::Status_Regsiter_Access: Status_Register_Access(ir); break;
	case AInstructionType::Load_Store: Load_Store(ir); break;
	case AInstructionType::Load_Store_Multiple: Load_Store_Multiple(ir); break;
	case AInstructionType::Semaphore: throw std::string("Unimplemented opcode"); break;
	case AInstructionType::Exception_Generating: Exception_Generating(ir); break;
	case AInstructionType::Coprocessor: throw std::string("Unimplemented opcode"); break;
	case AInstructionType::Extensions: throw std::string("Unimplemented opcode"); break;
	}
}

inline void CPU::Exception_Generating(IR_ARM& ir) {
	switch (ir.instr) {
	case AInstructions::SWI:
		if (swi_callback) {
			swi_callback(ir.operand1);

			//return after swi is done
			//same as BX LR
			cpsr.flag_T = !!(gprs[Regs::LR] & 0b1);
			gprs[Regs::PC] = gprs[Regs::LR] & 0xFFFFFFFE;
		}
		else {
			throw std::string("callback not set for swi");
		}
		break;
	}
}

inline void CPU::Load_Store_Multiple(IR_ARM& ir) {
	u32& reg_list = ir.operand1;
	u32& Rn = ir.operand2;

	bool P = (ir.operand3 & 0b1000) >> 3 == 1;
	bool U = (ir.operand3 & 0b0100) >> 2 == 1;
	bool S = (ir.operand3 & 0b0010) >> 1 == 1;
	bool W = (ir.operand3 & 0b0001) == 1;

	unsigned number_regs_modified = 0;

	for (int i = 0; i < 16; i++) {
		if (getBit(reg_list, i) == 1) {
			number_regs_modified++;
		}
	}

	u32 start_address = 0;
	u32 end_address = 0;

	switch (ir.instr) {
	case AInstructions::STMEA:
	case AInstructions::STMIA:
	case AInstructions::LDMFD:
	case AInstructions::LDMIA:
		start_address = gprs[Rn];
		end_address = gprs[Rn] + (number_regs_modified * 4) - 4;
		if (W == 1) {
			gprs[Rn] = gprs[Rn] + (number_regs_modified * 4);
		}
		break;

	case AInstructions::STMFA:
	case AInstructions::STMIB:
	case AInstructions::LDMED:
	case AInstructions::LDMIB:
		start_address = gprs[Rn] + 4;
		end_address = gprs[Rn] + (number_regs_modified * 4);
		if (W == 1) {
			gprs[Rn] = gprs[Rn] + (number_regs_modified * 4);
		}
		break;

	case AInstructions::STMED:
	case AInstructions::STMDA:
	case AInstructions::LDMFA:
	case AInstructions::LDMDA:
		start_address = gprs[Rn] - (number_regs_modified * 4) + 4;
		end_address = gprs[Rn];
		if (W == 1) {
			gprs[Rn] = gprs[Rn] - (number_regs_modified * 4);
		}
		break;

	case AInstructions::STMFD:
	case AInstructions::STMDB:
	case AInstructions::LDMEA:
	case AInstructions::LDMDB:
		start_address = gprs[Rn] - (number_regs_modified * 4);
		end_address = gprs[Rn] - 4;
		if (W == 1) {
			gprs[Rn] = gprs[Rn] - (number_regs_modified * 4);
		}
		break;
	}

	switch (ir.instr) {

		//Load
	case AInstructions::LDMFA: case AInstructions::LDMDA: case AInstructions::LDMFD: case AInstructions::LDMIA:
	case AInstructions::LDMEA: case AInstructions::LDMDB: case AInstructions::LDMED: case AInstructions::LDMIB:
	{
		//Doesn't do the second form of the LDM instructions
		//because other cpu mode are not emulated
		u32 address = start_address;
		for (int i = 0; i <= 14; i++) {
			if (getBit(reg_list, i) == 1) {
				gprs[i] = mem.read32(address);
				address = address + 4;
			}
		}
		if (S == 1) {
			cpsr = spsr;
		}
		if (getBit(reg_list, 15) == 1) {
			u32 value = mem.read32(address);
			gprs[Regs::PC] = value & 0xFFFFFFFC;
			address = address + 4;
		}
		if (!(end_address == address - 4)) {
			throw std::string("Something went wrong in Load multiple");
		}
		break;
	}
	//Store
	case AInstructions::STMED: case AInstructions::STMDA: case AInstructions::STMEA: case AInstructions::STMIA:
	case AInstructions::STMFD: case AInstructions::STMDB: case AInstructions::STMFA: case AInstructions::STMIB:
	{
		//Doesn't do the second form of the Store instructions
		//because other cpu mode are not emulated
		u32 address = start_address;
		for (int i = 0; i <= 15; i++) {
			if (getBit(reg_list, i) == 1) {
				mem.write32(address, gprs[i]);
				address = address + 4;
			}
		}
		if (!(end_address == address - 4)) {
			throw std::string("Something went wrong in Store multiple");
		}

		break;
	}
	}
}

inline void CPU::Load_Store(IR_ARM& ir) {
	u32& Rd = ir.operand1;
	u32& Rn = ir.operand2;

	bool P = (ir.operand3 & 0b1000) >> 3 == 1;
	bool U = (ir.operand3 & 0b0100) >> 2 == 1;
	bool W = (ir.operand3 & 0b0001) == 1;

	u32 offset;
	std::tie(offset, std::ignore) = shifter_operand(ir.shifter_operand, false);

	u32 address = 0;
	if (!P) {
		//post-indexed
		address = gprs[Rn];
		if (U) gprs[Rn] = gprs[Rn] + offset;
		else gprs[Rn] = gprs[Rn] - offset;
	}
	else if (P && W) {
		//pre-indexed
		if (U) gprs[Rn] = gprs[Rn] + offset;
		else gprs[Rn] = gprs[Rn] - offset;
		address = Rn;
	}
	else {
		//offset
		if (U) address = gprs[Rn] + offset;
		else address = gprs[Rn] - offset;
		if (Rn == Regs::PC) {
			//To change if the pc is incremented before the instruction is executed
			address += 8;
		}
	}

	/*
	For all T ending instructions:
	TODO: take care of translation. The arm arm is not very clear
	Should also access user mode register even not in user mode
	*/

	switch (ir.instr) {
	case AInstructions::LDRT:
	case AInstructions::LDR:
		gprs[Rd] = mem.read32(ror32(address, 8 * (address & 0b11)));
		if (Rd == Regs::PC) gprs[Rd] &= 0xFFFFFFFC;
		if (Rd == Regs::PC && (address & 0b11) != 0b00) throw std::string("unpredictable instructions are not emulated");
		break;

	case AInstructions::LDRBT:
	case AInstructions::LDRB:
		if (Rd == Regs::PC) throw std::string("unpredictable instructions are not emulated");
		gprs[Rd] = mem.read8(address);
		break;

	case AInstructions::STRT:
	case AInstructions::STR:
		if (Rd == Regs::PC) throw std::string("implementation defined instructions are not emulated");
		mem.write32(address, gprs[Rd]);
		break;

	case AInstructions::STRBT:
	case AInstructions::STRB:
		if (Rd == Regs::PC) throw std::string("unpredictable instructions are not emulated");
		mem.write8(address, gprs[Rd] & 0xFF);
		break;

	case AInstructions::LDRH:
		if (Rd == Regs::PC || (address & 0b1) == 1) throw std::string("unpredictable instructions are not emulated");
		gprs[Rd] = mem.read16(address);
		break;

	case AInstructions::STRH:
		if (Rd == Regs::PC || (address & 0b1) == 1) throw std::string("unpredictable instructions are not emulated");
		mem.write16(address, gprs[Rd] & 0xFFFF);
		break;

	case AInstructions::LDRSB:
		if (Rd == Regs::PC) throw std::string("unpredictable instructions are not emulated");
		gprs[Rd] = SignExtend<s32>(mem.read8(address), 8);
		break;

	case AInstructions::LDRSH:
		if (Rd == Regs::PC || (address & 0b1) == 1) throw std::string("unpredictable instructions are not emulated");
		gprs[Rd] = SignExtend<s32>(mem.read16(address), 16);
		break;
	}
}

inline void CPU::Status_Register_Access(IR_ARM& ir) {
	u32& R = ir.operand1;

	if (ir.instr == AInstructions::MRS) {
		u32 Rd = ir.operand2;

		if (Rd == Regs::PC) {
			throw std::string("Unpredictable instructions are not emulated");
		}

		if (R) {
			if (cpsr.mode == CpuMode::User || cpsr.mode == CpuMode::System) {
				throw std::string("Unpredictable, there is no spsr");
			}
			gprs[Rd] = PSR_to_u32(spsr);
		}
		else {
			gprs[Rd] = PSR_to_u32(cpsr);
		}
	}
	else if (ir.instr == AInstructions::MSR) {
		unsigned field_mask = ir.operand2;

		u32 shifter_op;
		std::tie(shifter_op, std::ignore) = shifter_operand(ir.shifter_operand, false);

		if (R) {
			if (cpsr.mode == CpuMode::User || cpsr.mode == CpuMode::System) {
				throw std::string("Unpredictable, there is no spsr");
			}
			throw std::string("MSR for priviledged modes not implemented yet.");
		}
		else {
			u32 new_CPSR = PSR_to_u32(cpsr);
			if (getBit(field_mask, 0) == 1 && cpsr.mode != CpuMode::User) {
				new_CPSR = (new_CPSR & 0xFFFFFF00) | (shifter_op & 0x000000FF);
			}
			if (getBit(field_mask, 1) == 1 && cpsr.mode != CpuMode::User) {
				new_CPSR = (new_CPSR & 0xFFFF00FF) | (shifter_op & 0x0000FF00);
			}
			if (getBit(field_mask, 2) == 1 && cpsr.mode != CpuMode::User) {
				new_CPSR = (new_CPSR & 0xFF00FFFF) | (shifter_op & 0x00FF0000);
			}
			if (getBit(field_mask, 3) == 1) {
				new_CPSR = (new_CPSR & 0x00FFFFFF) | (shifter_op & 0xFF000000);
			}
			u32_to_PSR(new_CPSR, cpsr);
		}
	}
}

inline void CPU::Multiply(IR_ARM& ir) {
	u32& Rm = ir.operand1;
	u32& Rs = ir.operand2;
	u32& Rn = ir.operand3;
	u32& Rd = ir.operand4;
	u32& RdLo = ir.operand3;
	u32& RdHi = ir.operand4;


	if (Rm == Regs::PC || Rs == Regs::PC || Rd == Regs::PC || Rn == Regs::PC) {
		throw std::string("Unpredictable instructions are not emulated");
	}

	/*
	TODO: Check if there is support for early termination. From page 167:
	"If the multiplier implementation supports early termination, it must be implemented
	on the value of the <Rs> operand. The type of early termination used (signed or
	unsigned) is IMPLEMENTATION DEFINED."
	*/

	// C and V flags should be UNPREDICTABLE in armv4T but are unchanged in armv5 and after.
	// Here they are treated as unchanged to avoid having to track their potential use in further
	// instructions

	switch (ir.instr) {
	case AInstructions::MUL: MUL_Instr1(ir.s, Rd, getLo(static_cast<u64>(Rm) * static_cast<u64>(Rs))); break;
	case AInstructions::MLA: MUL_Instr1(ir.s, Rd, getLo(static_cast<u64>(Rm) * static_cast<u64>(Rs)) + Rn); break;

	case AInstructions::UMULL: {
		u64 result = static_cast<u64>(Rm) * static_cast<u64>(Rs);
		MUL_Instr2(ir.s, RdHi, RdLo, getHi(result), getLo(result));
		break;
	}

	case AInstructions::UMLAL: {
		u64 result = static_cast<u64>(Rm) * static_cast<u64>(Rs);
		MUL_Instr2(ir.s, RdHi, RdLo, getHi(result) + gprs[RdHi] + CarryFrom(getLo(result), gprs[RdLo]), getLo(result) + gprs[RdLo]);
		break;
	}

	case AInstructions::SMULL: {
		s64 result = static_cast<s64>(Rm) * static_cast<s64>(Rs);
		MUL_Instr2(ir.s, RdHi, RdLo, getHi(result), getLo(result));
		break;
	}

	case AInstructions::SMLAL: {
		s64 result = static_cast<s64>(Rm) * static_cast<s64>(Rs);
		MUL_Instr2(ir.s, RdHi, RdLo, getHi(result) + gprs[RdHi] + CarryFrom(getLo(result), gprs[RdLo]), getLo(result) + gprs[RdLo]);
		break;
	}

	}
}

inline void CPU::MUL_Instr1(bool S, unsigned Rd, u32 result) {
	gprs[Rd] = result;
	if (S) {
		cpsr.flag_N = !!getBit(gprs[Rd], 31);
		cpsr.flag_Z = gprs[Rd] == 0;
	}
}

inline void CPU::MUL_Instr2(bool S, unsigned RdHi, unsigned RdLo, u32 resultHi, u32 resultLo) {
	gprs[RdHi] = resultHi;
	gprs[RdLo] = resultLo;
	if (S) {
		cpsr.flag_N = !!getBit(gprs[RdHi], 31);
		cpsr.flag_Z = (gprs[RdHi] == 0) & (gprs[RdLo] == 0);
	}
}

inline void CPU::Branch(IR_ARM& ir) {
	switch (ir.instr) {
	case AInstructions::B:
		gprs[Regs::PC] += SignExtend<s32>(ir.operand1 << 2, 26) + 8;
		break;

	case AInstructions::BL:
		gprs[Regs::LR] = gprs[Regs::PC] + 4;
		gprs[Regs::PC] += SignExtend<s32>(ir.operand1 << 2, 26) + 8;
		break;

	case AInstructions::BX:
		cpsr.flag_T = !!(gprs[ir.operand1] & 0b1);
		gprs[Regs::PC] = gprs[ir.operand1] & 0xFFFFFFFE;
		break;
	}
}

inline void CPU::Data_Processing(IR_ARM& ir) {
	u32& Rd = ir.operand1;
	u32& Rn = ir.operand2;
	u32 shifter_op;
	bool shifter_carry;
	std::tie(shifter_op, shifter_carry) = shifter_operand(ir.shifter_operand, false);

	std::function<bool()> fun_Rd_31 = [&]()->bool {return !!getBit(gprs[Rd], 31); };
	std::function<bool()> fun_Rd_0 = [&]()->bool {return gprs[Rd] == 0; };
	std::function<bool()> fun_shifter_carry = [&]()->bool{return shifter_carry; };

	switch (ir.instr) {
	case AInstructions::AND: DP_Instr1(ir.s, Rd, gprs[Rn] & shifter_op, fun_Rd_31, fun_Rd_0, fun_shifter_carry, fun_V); break;
	case AInstructions::EOR: DP_Instr1(ir.s, Rd, gprs[Rn] ^ shifter_op, fun_Rd_31, fun_Rd_0, fun_shifter_carry, fun_V); break;
	case AInstructions::SUB: DP_Instr1(ir.s, Rd, gprs[Rn] - shifter_op, fun_Rd_31, fun_Rd_0, [&]()->bool {return !BorrowFromSub(gprs[Rn], shifter_op); }, [&]()->bool {return OverflowFromSub(gprs[Rn], shifter_op); }); break;
	case AInstructions::RSB: DP_Instr1(ir.s, Rd, shifter_op - gprs[Rn], fun_Rd_31, fun_Rd_0, [&]()->bool {return !BorrowFromSub(shifter_op, gprs[Rn]); }, [&]()->bool {return OverflowFromSub(shifter_op, gprs[Rn]); }); break;
	case AInstructions::ADD: DP_Instr1(ir.s, Rd, gprs[Rn] + shifter_op, fun_Rd_31, fun_Rd_0, [&]()->bool {return CarryFrom(gprs[Rn], shifter_op); }, [&]()->bool {return OverflowFromAdd(gprs[Rn], shifter_op); }); break;
	case AInstructions::ADC: DP_Instr1(ir.s, Rd, gprs[Rn] + shifter_op + cpsr.flag_C, fun_Rd_31, fun_Rd_0, [&]()->bool {return CarryFrom(gprs[Rn], shifter_op, cpsr.flag_C); }, [&]()->bool {return OverflowFromAdd(gprs[Rn], shifter_op, cpsr.flag_C); }); break;
	case AInstructions::SBC: DP_Instr1(ir.s, Rd, gprs[Rn] - shifter_op - !cpsr.flag_C, fun_Rd_31, fun_Rd_0, [&]()->bool {return !BorrowFromSub(gprs[Rn], shifter_op, !cpsr.flag_C); }, [&]()->bool {return OverflowFromSub(gprs[Rn], shifter_op, !cpsr.flag_C); }); break;
	case AInstructions::RSC: DP_Instr1(ir.s, Rd, shifter_op - gprs[Rn] - !cpsr.flag_C, fun_Rd_31, fun_Rd_0, [&]()->bool {return !BorrowFromSub(shifter_op, gprs[Rn], !cpsr.flag_C); }, [&]()->bool {return OverflowFromSub(shifter_op, gprs[Rn], !cpsr.flag_C); }); break;
	case AInstructions::TST: DP_Instr2(gprs[Rn] & shifter_op, fun_r_31, fun_r_0, [&](u32 r)->bool {return shifter_carry; }, fun_r_V); break;
	case AInstructions::CMP: DP_Instr2(gprs[Rn] - shifter_op, fun_r_31, fun_r_0, [&](u32 r)->bool {return !BorrowFromSub(gprs[Rn], shifter_op); }, [&](u32 r)->bool {return OverflowFromSub(gprs[Rn], shifter_op); }); break;
	case AInstructions::CMN: DP_Instr2(gprs[Rn] + shifter_op, fun_r_31, fun_r_0, [&](u32 r)->bool {return CarryFrom(gprs[Rn], shifter_op); }, [&](u32 r)->bool {return OverflowFromAdd(gprs[Rn], shifter_op); }); break;
	case AInstructions::TEQ: DP_Instr2(gprs[Rn] ^ shifter_op, fun_r_31, fun_r_0, [&](u32 r)->bool {return shifter_carry; }, fun_r_V); break;
	case AInstructions::ORR: DP_Instr1(ir.s, Rd, gprs[Rn] | shifter_op, fun_Rd_31, fun_Rd_0, fun_shifter_carry, fun_V); break;
	case AInstructions::MOV: DP_Instr1(ir.s, Rd, shifter_op, fun_Rd_31, fun_Rd_0, fun_shifter_carry, fun_V); break;
	case AInstructions::BIC: DP_Instr1(ir.s, Rd, gprs[Rn] & ~shifter_op, fun_Rd_31, fun_Rd_0, fun_shifter_carry, fun_V); break;
	case AInstructions::MVN: DP_Instr1(ir.s, Rd, ~shifter_op, fun_Rd_31, fun_Rd_0, fun_shifter_carry, fun_V); break;
	//TODO verify negation for MVN and BIC
	}
}

std::tuple<u32, bool> CPU::shifter_operand(Shifter_op& so, bool negatif) {

	u32& Rm = so.operand1;
	u32& Rs = so.operand2;
	u32& shift_imm = so.operand2;

	//TODO take care of PC as Rm, Rn, Rd, Rs
	if (Rm == Regs::PC || (Rs == Regs::PC && so.type != Shifter_type::Immediate && so.type != Shifter_type::Register && so.type != Shifter_type::LSL_imm && so.type != Shifter_type::LSR_imm && so.type != Shifter_type::ASR_imm && so.type != Shifter_type::ROR_imm && so.type != Shifter_type::RRX)) {
		throw std::string("unimpletend: PC as Rm, Rn, Rd, Rs is either unpredictable or current instruction address + 8");
	}

	unsigned vRs7_0 = gprs[Rs] & 0xFF;
	unsigned vRs4_0 = gprs[Rs] & 0b11111;

	switch (so.type) {
	case Shifter_type::Immediate:
		if (so.operand2 == 0) return std::make_tuple(so.operand1, cpsr.flag_C);
		else return std::make_tuple(so.operand1, getBit(so.operand1, 31) == 1);

	case Shifter_type::Register: return std::make_tuple(gprs[Rm], cpsr.flag_C);

	case Shifter_type::LSL_imm: return std::make_tuple(gprs[Rm] << shift_imm, getBit(gprs[Rm], 32 - shift_imm) == 1);

	case Shifter_type::LSL_reg:
		if (vRs7_0 == 0) return std::make_tuple(gprs[Rm], cpsr.flag_C);
		else if (vRs7_0 < 32) return std::make_tuple(gprs[Rm] << vRs7_0, getBit(gprs[Rm], 32 - vRs7_0) == 1);
		else if (vRs7_0 == 32) return std::make_tuple(0, (gprs[Rm] & 0b1) == 1);
		else return std::make_tuple(0, false);

	case Shifter_type::LSR_imm: //>> is logical because gprs are unsigned
		if (shift_imm == 32) return std::make_tuple(0, getBit(gprs[Rm], 31) == 1);
		else return std::make_tuple(gprs[Rm] >> shift_imm, getBit(gprs[Rm], shift_imm - 1) == 1);

	case Shifter_type::LSR_reg:
		if (vRs7_0 == 0) return std::make_tuple(gprs[Rm], cpsr.flag_C);
		else if (vRs7_0 < 32) return std::make_tuple(gprs[Rm] >> vRs7_0, getBit(gprs[Rm], vRs7_0 - 1) == 1);
		else if (vRs7_0 == 32) return std::make_tuple(0, getBit(gprs[Rm], 31) == 1);
		else return std::make_tuple(0, false);

	case Shifter_type::ASR_imm:
		if (shift_imm == 32)
			if (getBit(gprs[Rm], 31) == 0) return std::make_tuple(0, getBit(gprs[Rm], 31) == 1);
			else return std::make_tuple(0xFFFFFFFF, getBit(gprs[Rm], 31) == 1);
		else return std::make_tuple(static_cast<signed>(gprs[Rm]) >> shift_imm, getBit(gprs[Rm], shift_imm - 1) == 1);

	case Shifter_type::ASR_reg:
		if (vRs7_0 == 0) return std::make_tuple(gprs[Rm], cpsr.flag_C);
		else if (vRs7_0 < 32) return std::make_tuple(static_cast<signed>(gprs[Rm]) >> vRs7_0, getBit(gprs[Rm], vRs7_0 - 1) == 1);
		else if (getBit(gprs[Rm], 31) == 0) return std::make_tuple(0, getBit(gprs[Rm], 31) == 1);
		else return std::make_tuple(0xFFFFFFFF, getBit(gprs[Rm], 31) == 1);

	case Shifter_type::ROR_imm: return std::make_tuple(ror32(gprs[Rm], shift_imm), getBit(gprs[Rm], shift_imm - 1) == 1);

	case Shifter_type::ROR_reg:
		if (vRs7_0 == 0) return std::make_tuple(gprs[Rm], cpsr.flag_C);
		else if (vRs4_0 == 0) return std::make_tuple(gprs[Rm], getBit(gprs[Rm], 31) == 1);
		else return std::make_tuple(ror32(gprs[Rm], vRs4_0), getBit(gprs[Rm], vRs4_0 - 1) == 1);

	case Shifter_type::RRX: return std::make_tuple((cpsr.flag_C << 31) | (gprs[Rm] >> 1), (gprs[Rm] & 0b1) == 1);

	default: throw "invalid shifter operand";
	}

}
