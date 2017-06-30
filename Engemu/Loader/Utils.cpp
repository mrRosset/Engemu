#include <string>
#include <iostream>
#include <fstream>

#include "Utils.h"

uint16_t utils::crc16_ccitt(uint8_t(&values)[6])
{
	uint32_t result = 0;

	for (uint8_t b : values)
	{
		result ^= (b << 8);

		for (uint8_t d = 0; d < 8; d++)
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

bool utils::loadData(std::string path, std::vector<uint8_t>& data) {

	std::ifstream stream(path, std::ios::binary);

	if (!stream)
	{
		std::cerr << "Failed to open image file." << std::endl;
		return false;
	}

	stream.seekg(0, std::ios::end);
	uint64_t length = stream.tellg();
	stream.seekg(0, std::ios::beg);

	data.resize(length);

	for (uint64_t i = 0; i < length; i++)
	{
		data[i] = stream.get();
	}

	stream.close();
	return true;
}

void utils::u8_from_32(uint8_t output[4], uint32_t value)
{
	output[0] = (uint8_t)value;
	output[1] = (uint8_t)(value >>= 8);
	output[2] = (uint8_t)(value >>= 8);
	output[3] = (uint8_t)(value >>= 8);
}
