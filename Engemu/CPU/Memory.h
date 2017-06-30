#pragma once

#include <vector>
#include "../Common.h"

class Memory {
public:
	std::vector<u8> mem;

	Memory() : mem(0x100) {}

	inline u8 read8(u32 address)
	{
		return mem[address];
	}

	inline u16 read16(u32 address)
	{
		return (read8(address + 1) << 8) | read8(address);
	}

	inline u32 read32(u32 address)
	{
		return (read16(address + 2) << 16) | read16(address);
	}

	inline void write8(u32 address, u8 value)
	{
		mem[address] = value;
	}

	inline void write16(u32 address, u16 value)
	{
		write8(address + 1, value >> 8);
		write8(address, value & 0xFF);
	}

	inline void write32(u32 address, u32 value)
	{
		write16(address + 2, value >> 16);
		write16(address, value & 0xFFFF);
	}


};