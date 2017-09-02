#include <iostream>
#include "../Common.h"
#include "../Decoder/Decoder.h"
#include "../Disassembler/Disassembler.h"
#include "../Utils.h"

#include "../catch/catch.hpp"


std::string Disassemble(u32 instr) {
	IR_ARM ir;
	Decoder::Decode(ir, instr);
	return Disassembler::Disassemble(ir);
}

TEST_CASE("Disassemble Data Processing instr", "[Disassembler][ARM]") {
	REQUIRE(Disassemble(0x2201230F) == "andcs r2, r1, #1006632960");
	REQUIRE(Disassemble(0xA0731048) == "rsbsge r1, r3, r8, asr #32");
}

TEST_CASE("Disassemble Branch instr", "[Disassembler][ARM]") {
	REQUIRE(Disassemble(0xEA000000) == "b +#8");
	REQUIRE(Disassemble(0xEAFFFFFE) == "b +#0");
	REQUIRE(Disassemble(0xEA000076) == "b +#480");
	REQUIRE(Disassemble(0xEB000000) == "bl +#8");
	REQUIRE(Disassemble(0xEBFFFFFE) == "bl +#0");
	REQUIRE(Disassemble(0xEB000076) == "bl +#480");
	REQUIRE(Disassemble(0x312FFF15) == "bxcc r5");
	REQUIRE(Disassemble(0x312FFF1E) == "bxcc lr");
	
	REQUIRE(Disassemble(0xEB000344) == "bl +#3352");
	REQUIRE(Disassemble(0xEB000248) == "bl +#2344");
}

TEST_CASE("Disassemble Status Register access instr", "[Disassembler][ARM]") {
	REQUIRE(Disassemble(0xE14F3000) == "mrs r3, SPSR");
	REQUIRE(Disassemble(0xE10FD000) == "mrs sp, CPSR");

	REQUIRE(Disassemble(0xE16BF003) == "msr SPSR_fxc, r3");
	REQUIRE(Disassemble(0xE12BF005) == "msr CPSR_fxc, r5");
	REQUIRE(Disassemble(0xE365F603) == "msr SPSR_sc, #3145728");
	REQUIRE(Disassemble(0xE320FA23) == "msr CPSR_, #143360");
}

TEST_CASE("Disassemble Load/Store unsigned Byte/Word", "[Disassembler][ARM]") {
	REQUIRE(Disassemble(0xE5331048) == "ldr r1, [r3, #-72]!");
	REQUIRE(Disassemble(0xE5B31048) == "ldr r1, [r3, #72]!");
	REQUIRE(Disassemble(0xE4331048) == "ldrt r1, [r3], #-72");
	REQUIRE(Disassemble(0xE4B31048) == "ldrt r1, [r3], #72");
	REQUIRE(Disassemble(0xE4731048) == "ldrbt r1, [r3], #-72");
	REQUIRE(Disassemble(0xE4F31048) == "ldrbt r1, [r3], #72");
	REQUIRE(Disassemble(0xE4531048) == "ldrb r1, [r3], #-72");
	REQUIRE(Disassemble(0xE4D31048) == "ldrb r1, [r3], #72");
	REQUIRE(Disassemble(0xE5931048) == "ldr r1, [r3, #72]");
	REQUIRE(Disassemble(0xE5D31048) == "ldrb r1, [r3, #72]");
	REQUIRE(Disassemble(0xE4731000) == "ldrbt r1, [r3], #-0");

	REQUIRE(Disassemble(0xE5883000) == "str r3, [r8, #0]");
}


TEST_CASE("Disassemble Load/Store signed Byte/ half-word", "[Disassembler][ARM]") {
	//Some of thow have P=0 and W=1 which is unpredictable, those should be removed
	REQUIRE(Disassemble(0xc01130b5) == "ldrhgt r3, [r1], -r5");
	REQUIRE(Disassemble(0xc05130b5) == "ldrhgt r3, [r1], #-5");
	REQUIRE(Disassemble(0xc08130b5) == "strhgt r3, [r1], r5");
	REQUIRE(Disassemble(0xc09130b5) == "ldrhgt r3, [r1], r5");
	REQUIRE(Disassemble(0xc0c130b5) == "strhgt r3, [r1], #5");
	REQUIRE(Disassemble(0xc0d130b5) == "ldrhgt r3, [r1], #5");
	REQUIRE(Disassemble(0xc10130b5) == "strhgt r3, [r1, -r5]");
	REQUIRE(Disassemble(0xc11130b5) == "ldrhgt r3, [r1, -r5]");
	REQUIRE(Disassemble(0xc12130b5) == "strhgt r3, [r1, -r5]!");
	REQUIRE(Disassemble(0xc13130b5) == "ldrhgt r3, [r1, -r5]!");
	REQUIRE(Disassemble(0xc14130b5) == "strhgt r3, [r1, #-5]");
	REQUIRE(Disassemble(0xc15130b5) == "ldrhgt r3, [r1, #-5]");
	REQUIRE(Disassemble(0xc16130b5) == "strhgt r3, [r1, #-5]!");
	REQUIRE(Disassemble(0xc17130b5) == "ldrhgt r3, [r1, #-5]!");
	REQUIRE(Disassemble(0xc18130b5) == "strhgt r3, [r1, r5]");
	REQUIRE(Disassemble(0xc19130b5) == "ldrhgt r3, [r1, r5]");
	REQUIRE(Disassemble(0xc1a130b5) == "strhgt r3, [r1, r5]!");
	REQUIRE(Disassemble(0xc1b130b5) == "ldrhgt r3, [r1, r5]!");
	REQUIRE(Disassemble(0xc1c130b5) == "strhgt r3, [r1, #5]");
	REQUIRE(Disassemble(0xc1d130b5) == "ldrhgt r3, [r1, #5]");
	REQUIRE(Disassemble(0xc1e130b5) == "strhgt r3, [r1, #5]!");
	REQUIRE(Disassemble(0xc1f130b5) == "ldrhgt r3, [r1, #5]!");
}

TEST_CASE("Disassemble Load/Store signed Multiple", "[Disassembler][ARM]") {
	REQUIRE(Disassemble(0xE8BD4070) == "ldmfd sp!, {r4-r6,lr}");
	REQUIRE(Disassemble(0xE92D4070) == "stmfd sp!, {r4-r6,lr}");
}

TEST_CASE("Disassemble Semaphore", "[Disassembler][ARM]") {
	REQUIRE(Disassemble(0xE10A5098) == "swp r5, r8, [r10]");
	REQUIRE(Disassemble(0xE1031092) == "swp r1, r2, [r3]");
	REQUIRE(Disassemble(0xE14A509C) == "swpb r5, r12, [r10]");
	REQUIRE(Disassemble(0xE1431092) == "swpb r1, r2, [r3]");
}

TEST_CASE("Disassemble SWI", "[Disassembler][ARM]") {
	REQUIRE(Disassemble(0xEFB3B3B3) == "swi #11776947");
	REQUIRE(Disassemble(0xEF5CB35C) == "swi #6075228");
	REQUIRE(Disassemble(0xEF00006C) == "swi #108");
}