#pragma once

#include "../Memory.h"


class TestMemory : public Memory {
public:
	std::vector<u8> ram;
	u32 ram_cursor;

	class TestMemory() : ram(0x1000'0000) {
		ram_cursor = 0;
	}

	inline u8 read8(u32 address) override {
		if (address < ram.size()) return ram[address];
		else throw std::string("Invalid read to unmapped memory");
	}

	inline void write8(u32 address, u8 value) override {
		if (address < ram.size()) ram[address] = value;
		else throw std::string("Invalid write to unmapped memory");
	}

	u32 allocateRam(u32 size) override {
		ram_cursor += size;
		return 0x6000'0000 + ram_cursor;
	}

};