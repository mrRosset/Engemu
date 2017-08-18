#include <string>
#include <iostream>
#include <fstream>

#include "Utils.h"

u16 utils::crc16_ccitt(u8(&values)[6])
{
	u32 result = 0;

	for (u8 b : values)
	{
		result ^= (b << 8);

		for (u8 d = 0; d < 8; d++)
		{
			result = result << 1;

			if (result & 0x10000)
			{
				result ^= 0x1021;
			}

			result &= 0xffff;
		}
	}

	return result;
}

bool utils::loadData(std::string& path, std::vector<u8>& data) {

	std::ifstream stream(path, std::ios::binary | std::ios::ate);

	if (!stream)
	{
		std::cerr << "Failed to open image file." << std::endl;
		return false;
	}

	u64 length = stream.tellg();
	data.resize(length);

	stream.seekg(0, std::ios::beg);

	if (!stream.read((char*)data.data(), length))
	{
		throw std::string("Error reading bytes from file");
	}

	stream.close();
	return true;
}

void utils::u8_from_32(u8 output[4], u32 value)
{
	output[0] = (u8)value;
	output[1] = (u8)(value >>= 8);
	output[2] = (u8)(value >>= 8);
	output[3] = (u8)(value >>= 8);
}
