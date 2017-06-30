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

bool utils::loadData(std::string path, std::vector<u8>& data) {

	std::ifstream stream(path, std::ios::binary);

	if (!stream)
	{
		std::cerr << "Failed to open image file." << std::endl;
		return false;
	}

	stream.seekg(0, std::ios::end);
	u64 length = stream.tellg();
	stream.seekg(0, std::ios::beg);

	data.resize(length);

	for (u64 i = 0; i < length; i++)
	{
		data[i] = stream.get();
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
