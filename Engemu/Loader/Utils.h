#pragma once
#include <vector>
#include "../Common.h"

namespace utils {
	u16 crc16_ccitt(u8(&values)[6]);
	bool loadData(std::string& path, std::vector<u8>& data);
	void u8_from_32(u8 output[4], u32 value);

	template <class T>
	inline bool getBit(T v, unsigned bit_number) {
		return ((v >> bit_number) & 0b1);
	}
	template <class T>
	inline bool getBit(T v, u64 bit_number) {
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