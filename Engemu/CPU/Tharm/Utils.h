#pragma once

#include <string>
#include <intrin.h>  
#include "../../Common.h"
#include "CPU.h"

/*
Code in this file is compiler-specific.
Some functions for circular functions are only defined for VC++ and GCC.
Also the operator >> is used. This operator is can be logical or arithmetic
In VC++ it's arithmetic. https://msdn.microsoft.com/en-us/library/336xbhcz.aspx
Not tested in any other compiler,
*/

#if defined(__GNUG__)
inline u8 ror8(const u8 x, const u8 n) {
	u8 result = x;
	__asm__("rorb %[n], %[result]" : [result] "+g" (result) : [n] "c" (n));
	return result;
}

inline u16 ror16(const u16 x, const u16 n) {
	u16 result = x;
	__asm__("rorw %b[n], %[result]" : [result] "+g" (result) : [n] "c" (n));
	return result;
}

inline u32 ror32(const u32 x, const u32 n) {
	u32 result = x;
	__asm__("rorl %b[n], %[result]" : [result] "+g" (result) : [n] "c" (n));
	return result;
}

inline u64 ror64(const u64 x, const u64 n) {
	u64 result = x;
	__asm__("rorq %b[n], %[result]" : [result] "+g" (result) : [n] "c" (n));
	return result;
}
#elif defined(_MSC_VER)
inline u8 ror8(const u8 x, const u8 n) { return _rotr8(x, n); }
inline u16 ror16(const u16 x, const u16 n) { return _rotr16(x, n % 16); }
inline u32 ror32(const u32 x, const u32 n) { return _rotr(x, n); }
inline u64 ror64(const u64 x, const u64 n) { return _rotr64(x, n % 64); }
#endif

template <class T>
inline unsigned getBit(T v, unsigned bit_number) {
	return ((v >> bit_number) & 0b1);
}
template <class T>
inline unsigned getBit(T v, u64 bit_number) {
	return ((v >> bit_number) & 0b1);
}
template <class T>
inline unsigned getBit(T v, signed bit_number) {
	if (bit_number < 0) throw std::string("Unimplemented opcode");
	return ((v >> bit_number) & 0b1);
}
//avoid implicit conversions
template <class T, class U>
unsigned getBit(T, U) = delete;


template<typename T>
inline T SignExtend(const T v, unsigned bit_count) {
	if (bit_count > sizeof(T) * 8) throw std::string("cannot sign extend to a type of smaller size");
	T mask = static_cast<T>(1ULL << bit_count) - 1;
	unsigned s = getBit(v, bit_count - 1);
	if (s == 1) {
		return v | ~mask;
	}
	return v;
}

inline bool CarryFrom(u64 a, u64 b) {
	return (a + b) > UINT32_MAX;
}
inline bool CarryFrom(u64 a, u64 b, u64 c) {
	return (a + b + c) > UINT32_MAX;
}

//TODO: check no sign extension
inline bool OverflowFromAdd(s32 a, s32 b) {
	s32 r = a + b;
	return (a > 0 && b > 0 && r < 0) || (a < 0 && b < 0 && r > 0);
}
inline bool OverflowFromAdd(s32 a, s32 b, s32 c) {
	s32 r = a + b + c;
	return (a > 0 && b > 0 && r < 0) || (a < 0 && b < 0 && r > 0);
}

inline bool OverflowFromSub(s32 a, s32 b) {
	s32 r = a - b;
	return (a > 0 && b < 0 && r < 0) || (a < 0 && b > 0 && r > 0);
}
inline bool OverflowFromSub(s32 a, s32 b, s32 c) {
	s32 r = a - b - c;
	return (a > 0 && b < 0 && r < 0) || (a < 0 && b > 0 && r > 0);
}

inline bool BorrowFromAdd(s32 a, s32 b) {
	return a + b < 0;
}
inline bool BorrowFromSub(u32 a, u32 b) {
	//To check
	return b > a;
}
inline bool BorrowFromSub(u32 a, u32 b, u32 c) {
	//To check
	return b + c > a;
}

inline u32 getHi(u64 result) {
	return (result >> 32) & 0xFFFFFFFF;
}

inline u32 getLo(u64 result) {
	return result & 0xFFFFFFFF;
}

inline s32 getHi(s64 result) {
	return (result >> 32) & 0xFFFFFFFF;
}

inline s32 getLo(s64 result) {
	return result & 0xFFFFFFFF;
}

//avoid implicit conversions
template <class T>
u32 getHi(T) = delete;

template <class T>
u32 getLo(T) = delete;


inline u32 PSR_to_u32(PSR& psr) {
	return (psr.flag_N << 31) | (psr.flag_Z << 30) | (psr.flag_C << 29) | (psr.flag_V << 28) |
		((psr.reserved & 0xFFFFF) << 8) | (psr.flag_inter_I << 7) | (psr.flag_inter_F << 6) |
		(psr.flag_T << 5) | (static_cast<u8>(psr.mode) & 0b11111);
}

inline void u32_to_PSR(u32 input, PSR& psr) {
	psr.mode = static_cast<CpuMode>(input & 0b11111);
	psr.flag_T = !!getBit(input, 5);
	psr.flag_inter_F = !!getBit(input, 6);
	psr.flag_inter_I = !!getBit(input, 7);
	psr.reserved = (input >> 8) & 0xFFFFF;
	psr.flag_V = !!getBit(input, 28);
	psr.flag_C = !!getBit(input, 29);
	psr.flag_Z = !!getBit(input, 30);
	psr.flag_N = !!getBit(input, 31);
}