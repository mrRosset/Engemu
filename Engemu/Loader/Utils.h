#pragma once
#include <stdint.h>
#include <vector>

namespace utils {
	uint16_t crc16_ccitt(uint8_t(&values)[6]);
	bool loadData(std::string path, std::vector<uint8_t>& data);
	void u8_from_32(uint8_t output[4], uint32_t value);

	template <class T>
	inline bool getBit(T v, unsigned bit_number) {
		return ((v >> bit_number) & 0b1);
	}
	template <class T>
	inline bool getBit(T v, uint64_t bit_number) {
		return ((v >> bit_number) & 0b1);
	}
	template <class T>
	inline bool getBit(T v, signed bit_number) {
		if (bit_number < 0) throw std::string("Unimplemented opcode");
		return ((v >> bit_number) & 0b1);
	}
	//avoid implicit conversions
	template <class T, class U>
	bool getBit(T, U) = delete;

}