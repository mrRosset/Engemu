#include <iostream>
#include <bitset>
#include "TestMemory.h"
#include "../Common.h"
#include "../CPU/CPU.h"
#include "../CPU/Utils.h"
#include "../CPU/Decoder/Decoder.h"
#include "../CPU/Disassembler/Disassembler.h"

#include <catch/catch.hpp>

/*
	Warning: All tests here were written by myself during developement of the
	emulator and were based on my understanding of the behavior on arm procesor.
	Those tests were not tried on any other emulator and any comprehension error
	of the reference manual will probably be present in those tests.
*/

TEST_CASE("Step without crashing", "[Global]") {
	TestMemory mem;
	CPU cpu(mem);
	cpu.Step();
}

struct condition_test {
	u32 cond;
	bool flag_N, flag_Z, flag_C, flag_V;
	bool should_execute;
};

static const std::vector<condition_test> condition_tests = {
	{ 0b0000, false, true , false, false, true  },
	{ 0b0000, false, false, false, false, false },
	{ 0b0001, false, false, false, false, true  },
	{ 0b0001, false, true , false, false, false },
	{ 0b0010, false, false, true , false, true  },
	{ 0b0010, false, false, false, false, false },
	{ 0b0011, false, false, false, false, true  },
	{ 0b0011, false, false, true , false, false },
	{ 0b0100, true , false, false, false, true  },
	{ 0b0100, false, false, false, false, false },
	{ 0b0101, false, false, false, false, true  },
	{ 0b0101, true , false, false, false, false },
	{ 0b0110, false, false, false, true , true  },
	{ 0b0110, false, false, false, false, false },
	{ 0b0111, false, false, false, false, true  },
	{ 0b0111, false, false, false, true , false },
	{ 0b1000, false, false, true , false, true  },
	{ 0b1000, false, true , true , false, false },
	{ 0b1000, false, false, false, false, false },
	{ 0b1000, false, true , false, true , false },
	{ 0b1001, false, true , false, false, true  },
	{ 0b1001, false, true , true , false, true  },
	{ 0b1001, false, false, false, false, true  },
	{ 0b1001, false, false, true , true , false },
	{ 0b1010, false, false, false, false, true  },
	{ 0b1010, true , false, false, true , true  },
	{ 0b1010, false, false, false, true , false },
	{ 0b1010, true , false, false, false, false },
	{ 0b1011, false, false, false, true , true  },
	{ 0b1011, true , false, false, false, true  },
	{ 0b1011, true , false, false, true , false },
	{ 0b1011, false, false, false, false, false },
	{ 0b1100, false, false, false, false, true  },
	{ 0b1100, true , false, false, true , true  },
	{ 0b1100, false, false, false, true , false },
	{ 0b1100, true , false, false, false, false },
	{ 0b1100, false, true , false, false, false },
	{ 0b1100, true , true , false, true , false },
	{ 0b1100, false, true , false, true , false },
	{ 0b1100, true , true , false, false, false },
	{ 0b1101, false, true , false, true , true  },
	{ 0b1101, true , true , false, false, true  },
	{ 0b1101, true , true , false, true , false },
	{ 0b1101, false, true , false, false, false },
	{ 0b1101, false, false, false, true , false },
	{ 0b1101, true , false, false, false, false },
	{ 0b1101, true , false, false, true , false },
	{ 0b1101, false, false, false, false, false },
	{ 0b1110, false, false, false, false, true  },
	{ 0b1110, true , true , true , true , true  },
};

TEST_CASE("Condition fields works correctly", "[ARM]") {
	//Rely on AND instruction working
	for (auto& test: condition_tests) {
		TestMemory mem;
		CPU cpu(mem);
		cpu.gprs[0] = 0b1100;
		cpu.gprs[1] = 0b1010;
		//and r0, r1, r0
		u32 op = 0b00000000000010000000000000000;

		cpu.cpsr.flag_N = test.flag_N;
		cpu.cpsr.flag_Z = test.flag_Z;
		cpu.cpsr.flag_C = test.flag_C;
		cpu.cpsr.flag_V = test.flag_V;
		//Global
		cpu.mem.write32(0, (test.cond << 28) | op);
		cpu.Step();
		if(test.should_execute)
			REQUIRE(cpu.gprs[0] == 0b1000);
		else 
			REQUIRE(cpu.gprs[0] == 0b1100);
	}

}

struct shifter_operand_32_imm_test {
	u32 rotate_imm;
	u32 immed_8;
	bool flag_C;
	u32 expected_result;
	bool expected_carry;
};

static const std::vector<shifter_operand_32_imm_test> shifter_operand_32_imm_tests = {
	//5.1.3 - Immediate
	{ 0xE, 0x3F, false, 0x3F0, false },
	{ 0xF, 0xFC, false, 0x3F0, false },
	{ 0, 0xAB, false, 0xAB, false },
	{ 0, 0xAB, true, 0xAB, true },
	{ 0x2, 0b1101, false, 0xD0000000, true },
	{ 0x1, 0b1101, false, 0x40000003, false },
};

TEST_CASE("Shifter Operand Immediate works correctly", "[ARM]") {
	for (auto& test : shifter_operand_32_imm_tests) {
		TestMemory mem;
		CPU cpu(mem);
		cpu.cpsr.flag_C = test.flag_C;
		//S = 1
		u32 op = 0b11100010000100000001000000000000;
		u32 opcode = (test.rotate_imm << 8) | test.immed_8 | op;
		IR_ARM ir;
		Decoder::Decode(ir, opcode);
		u32 shifter_op;
		bool shifter_carry;
		std::tie(shifter_op, shifter_carry) = cpu.shifter_operand(ir.shifter_operand, false);

		REQUIRE(shifter_op == test.expected_result);
		REQUIRE(shifter_carry == test.expected_carry);
	}
}

struct shifter_operand_imm_shifts_test {
	u32 shift_imm;
	u32 shift;
	u32 vRm;
	bool flag_C;
	u32 expected_result;
	bool expected_carry;
};

//TODO deal with Rn, Rm = PC (value of PC + 8)
static const std::vector<shifter_operand_imm_shifts_test> shifter_operand_imm_shifts_tests = {
	//5.1.4 - Register
	{ 0b0000, 0b00, 0x00000005, false, 0x00000005, false},
	{ 0b0000, 0b00, 0x00000005, true , 0x00000005, true },
	{ 0b0000, 0b00, 0x00000000, false, 0x00000000, false},
	{ 0b0000, 0b00, 0x00000000, true , 0x00000000, true },
	{ 0b0000, 0b00, 0xFFFFFFFF, false, 0xFFFFFFFF, false},
	{ 0b0000, 0b00, 0xFFFFFFFF, true , 0xFFFFFFFF, true },

	//5.1.5 - Logical shift left by immediate
	{ 0b0100, 0b00, 0xFFFFFFFF, false, 0xFFFFFFF0, true },
	{ 0b1111, 0b00, 0xFFFFFFFF, false, 0xFFFF8000, true },
	{ 0b1001, 0b00, 0x00000000, true , 0x00000000, false},
	{ 0b1001, 0b00, 0x00000003, true , 0x00000600, false},
	{ 0b0000, 0b00, 0xFFFFFFFF, false, 0xFFFFFFFF, false},

	//5.1.7 - Logical shift right by immediate
	{ 0b0100, 0b01, 0xFFFFFFFF, false, 0x0FFFFFFF, true },
	{ 0b1111, 0b01, 0xFFFFFFFF, false, 0x0001FFFF, true },
	{ 0b1001, 0b01, 0x00000000, false, 0x00000000, false},
	{ 0b0011, 0b01, 0xAFAFAFAF, false, 0x15F5F5F5, true },
	{ 0b0000, 0b01, 0xFFFFFFFF, false, 0x00000000, true },

	//5.1.9 - Arithmetic shift right by immediate
	{ 0b0100, 0b10, 0xFFFFFFFF, false, 0xFFFFFFFF, true },
	{ 0b0100, 0b10, 0xFFFFF0FF, false, 0xFFFFFF0F, true },
	{ 0b1001, 0b10, 0x00000000, false, 0x00000000, false},
	{ 0b0000, 0b10, 0x80000000, false, 0xFFFFFFFF, true },
	{ 0b0000, 0b10, 0x12345678, false, 0x00000000, false},

	//5.1.11 - Rotate right by immediate
	{ 0b0100, 0b11, 0xFFFFFFFF, false, 0xFFFFFFFF, true },
	{ 0b0100, 0b11, 0xFFFFF0FF, false, 0xFFFFFF0F, true },
	{ 0b1001, 0b11, 0x00000000, false, 0x00000000, false},
	{ 0b0100, 0b11, 0x80000007, false, 0x78000000, false},
	{ 0b0100, 0b11, 0x12345678, false, 0x81234567, true },

	//5.1.13 - Rotate right with extend
	{ 0b0000, 0b11, 0xFFFFFFFF, true , 0xFFFFFFFF, true },
	{ 0b0000, 0b11, 0xFFFFFFFF, false, 0x7FFFFFFF, true },
	{ 0b0000, 0b11, 0x12345678, false, 0x091A2B3C, false},
	{ 0b0000, 0b11, 0x12345678, true , 0x891A2B3C, false},

};

TEST_CASE("Shifter Operand Immediate shifts works correctly", "[ARM]") {
	for (auto& test : shifter_operand_imm_shifts_tests) {
		//TODO deal with Rn, Rm = PC (value of PC + 8)
		for (unsigned Rm = 0; Rm < Regs::PC; Rm++) {
			TestMemory mem;
			CPU cpu(mem);
			cpu.cpsr.flag_C = test.flag_C;
			cpu.gprs[Rm] = test.vRm;
			//S = 1
			u32 op = 0b11100000000100000001000000000000;
			u32 opcode = (test.shift_imm << 7) | (test.shift << 5) | Rm | op;
			IR_ARM ir;
			Decoder::Decode(ir, opcode);
			u32 shifter_op;
			bool shifter_carry;
			std::tie(shifter_op, shifter_carry) = cpu.shifter_operand(ir.shifter_operand, false);

			REQUIRE(shifter_op == test.expected_result);
			REQUIRE(shifter_carry == test.expected_carry);
		}
	}
}

struct shifter_operand_reg_shifts_test {
	u32 vRs;
	u32 shift;
	u32 vRm;
	bool flag_C;
	u32 expected_result;
	bool expected_carry;
};

static const std::vector<shifter_operand_reg_shifts_test> shifter_operand_reg_shifts_tests = {
	//5.1.6 - Logical shift left by register
	//{ 0x00000000, 0b00, 0x12345678, false , 0x12345678, false },
};

TEST_CASE("Shifter Operand Regs shifts works correctly", "[ARM]") {
	for (auto& test : shifter_operand_reg_shifts_tests) {
		for (unsigned Rm = 0; Rm < Regs::PC; Rm++) {
			for (unsigned Rs = 0; Rs < Regs::PC; Rs++) {
				TestMemory mem;
				CPU cpu(mem);
				cpu.cpsr.flag_C = test.flag_C;
				cpu.gprs[Rm] = test.vRm;
				cpu.gprs[Rs] = test.vRs;
				//S = 1
				u32 op = 0b11100000000100000001000000010000;
				u32 opcode = (Rs << 8) | (test.shift << 5) | Rm | op;
				IR_ARM ir;
				Decoder::Decode(ir, opcode);
				u32 shifter_op;
				bool shifter_carry;
				std::tie(shifter_op, shifter_carry) = cpu.shifter_operand(ir.shifter_operand, false);

				REQUIRE(shifter_op == test.expected_result);
				REQUIRE(shifter_carry == test.expected_carry);
			}
		}
	}
}


struct data_processing_test {
	AInstructions op;
	bool S;
	u32 shifter_result;
	u32 operand1;
	u32 operand2;
	u32 expected_result;
};

static const std::vector<data_processing_test> data_processing_tests = {
	{ AInstructions::ADD, false, 0x2cc, 10, 0, 0x2D6 },
	{ AInstructions::ADD, false, 53005, 2550, 0, 55555 },
};

TEST_CASE("Data Processing", "[ARM]") {
	for (auto& test : data_processing_tests) {
		TestMemory mem;
		CPU cpu(mem);
		IR_ARM instr = { AInstructionType::Data_Processing, test.op, Conditions::AL, test.S,  1, 0};
		instr.shifter_operand = { Shifter_type::Immediate, test.shifter_result, 0};
		cpu.gprs[0] = test.operand1;
		cpu.Execute(instr);
		REQUIRE(cpu.gprs[1] == test.expected_result);
	}
}

TEST_CASE("Simple AND test case", "[ARM]") {
	TestMemory mem;
	CPU cpu(mem);
	IR_ARM instr = { AInstructionType::Data_Processing, AInstructions::AND, Conditions::AL, true,  1, 0 };
	instr.shifter_operand = { Shifter_type::Immediate, 0b11010, 0 };	
	cpu.gprs[0] = 0b10110;
	cpu.cpsr.flag_N = true;
	cpu.cpsr.flag_Z = true;
	cpu.cpsr.flag_C = true;
	cpu.Execute(instr);
	REQUIRE(cpu.gprs[1] == 0b10010);
	REQUIRE(cpu.cpsr.flag_N == false);
	REQUIRE(cpu.cpsr.flag_Z == false);
	REQUIRE(cpu.cpsr.flag_C == true);
}
