#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "../Common.h"

/*
From THC
+----------------------------+------------------------------------------------+
| 0x0040 0000 - 0x2FFF FFFF  : User Data                                      |
| 0x3000 0000 - 0x3FFF FFFF  : Static data for Java                           |
| 0x4000 0000 - 0x4000 1FFF  : Super page + CPU page                          |
| 0x4001 0000 - 0x4001 0FFF  : Shadow RAM page temporary address              |
| 0x4100 0000 - 0x4100 3FFF  : Page Directory                                 |
| 0x4108 0000 - 0x4108 3FFF  : Page table info                                |
| 0x4200 0000 - 0x423F FFFF  : Page tables                                    |
| 0x5000 0000 - 0x57FF FFFF  : ROM image                                      |
| 0x5800 0000 - 0x5EFF FFFF  : Memory-mapped I/O (danger!)                    |
| 0x5F00 0000 - 0x5FFF FFFF  : Video RAM (fun!)                               |
| 0x6000 0000 - 0x7FFF FFFF  : RAM                                            |
| 0x8000 0000 - 0xXXXX XXXX  : Kernel data/bss section                        |
| 0xXXXX XXXX - 0xXXXX XXXX  : Reentrant/IRQ/FIQ/Null/Exception kernel stack  |
| 0xXXXX XXXX - 0xXXXX XXXX  : Fixed chunks data for ROM fixed processes (?)  |
| 0xXXXX XXXX - 0xXXXX XXXX  : Kernel server heap and stack                   |
| 0xXXXX XXXX - 0xXXXX XXXX  : Home Section / All Processes                   |
| 0xXXXX XXXX - 0xXXXX XXXX  : RAM-loaded EXE & DLL code                      |
| 0xFFF0 0000 - 0xFFFE FFFF  : Void / Empty                                   |
| 0xFFFF 0000 - 0xFFFF FFFF  : Vectors (hoho!)                                |
+----------------------------+------------------------------------------------+
*/


class Memory {
public:

	std::vector<u8> user_data;
	std::vector<u8> rom;
	std::vector<u8> ram;

	Memory() : user_data(0x2FFF'FFFF - 0x0040'0000), rom(0x57FF'FFFF - 0x5000'0000), ram(0x7FFF'FFFF - 0x6000'0000) {}

	void loadRom(std::string& rom_path) {
		std::ifstream stream(rom_path, std::ios::binary | std::ios::ate);

		if (!stream) {
			std::cerr << "Failed to open image file." << std::endl;
			return;
		}

		u64 length = stream.tellg();

		if (length > rom.size()) {
			std::cerr << "Rom dump is too big";
		}

		stream.seekg(0, std::ios::beg);
		
		if (!stream.read((char*)rom.data(), length))
		{
			throw std::string("Error reading bytes from file");
		}
		stream.close();
	}


	inline u8 read8(u32 address)
	{
		if (address >= 0x0040'0000 && address <= 0x2FFF'FFFF) return user_data[address - 0x0040'0000];
		else if (address >= 0x5000'0000 && address <= 0x57FF'FFFF) return rom[address - 0x5000'0000];
		else if (address >= 0x6000'0000 && address <= 0x7FFF'FFFF) return ram[address - 0x6000'0000];
		else throw (std::string("read to unmapped memory:") + std::to_string(address));
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
		if (address >= 0x0040'0000 && address <= 0x2FFF'FFFF) user_data[address - 0x0040'0000] = value;
		else if (address >= 0x5000'0000 && address <= 0x57FF'FFFF) rom[address - 0x5000'0000] = value;
		else if (address >= 0x6000'0000 && address <= 0x7FFF'FFFF) ram[address - 0x6000'0000] = value;
		else throw (std::string("write to unmapped memory:") + std::to_string(address));
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