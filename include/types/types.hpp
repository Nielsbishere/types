#pragma once
#include <stdint.h>

// Sized types

using i8    = int8_t;
using i16   = int16_t;
using i32   = int32_t;
using i64   = int64_t;

using u8    = uint8_t;
using u16   = uint16_t;
using u32   = uint32_t;
using u64   = uint64_t;
using usz	= size_t;

using f32   = float;
using f64   = double;

using c8	= char;
using c16	= char16_t;
using c32	= char32_t;

// Suffixes for casting to types

constexpr u8 operator ""_u8(unsigned long long test) { return u8(test); }
constexpr i8 operator ""_i8(unsigned long long test) { return i8(test); }
constexpr c8 operator ""_c8(unsigned long long test) { return c8(test); }

constexpr u16 operator ""_u16(unsigned long long test) { return u16(test); }
constexpr i16 operator ""_i16(unsigned long long test) { return i16(test); }
constexpr c16 operator ""_c16(unsigned long long test) { return c16(test); }

constexpr u32 operator ""_u32(unsigned long long test) { return u32(test); }
constexpr i32 operator ""_i32(unsigned long long test) { return i32(test); }
constexpr c32 operator ""_c32(unsigned long long test) { return c32(test); }

constexpr u64 operator ""_u64(unsigned long long test) { return u64(test); }
constexpr i64 operator ""_i64(unsigned long long test) { return i64(test); }

constexpr usz operator ""_usz(unsigned long long test) { return usz(test); }

constexpr f32 operator ""_f32(long double test) { return f32(test); }
constexpr f64 operator ""_f64(long double test) { return f64(test); }

// Maximum values

constexpr u8  u8_MAX	= 0xFF_u8;
constexpr u8  u8_MIN	= 0_u8;
constexpr u16 u16_MAX	= 0xFFFF_u16;
constexpr u16 u16_MIN	= 0_u16;
constexpr u32 u32_MAX	= 0xFFFFFFFF_u32;
constexpr u32 u32_MIN	= 0_u32;
constexpr u64 u64_MAX	= 0xFFFFFFFFFFFFFFFF_u64;
constexpr u64 u64_MIN	= 0_u64;

constexpr i8  i8_MAX	= 0x7F_i8;
constexpr i8  i8_MIN	= 0x80_i8;
constexpr c8  c8_MAX	= 0x7F_c8;
constexpr c8  c8_MIN	= 0x80_c8;
constexpr i16 i16_MAX	= 0x7FFF_i16;
constexpr i16 i16_MIN	= 0x8000_i16;
constexpr c16 c16_MAX	= 0x7FFF_c16;
constexpr c16 c16_MIN	= 0x8000_c16;
constexpr i32 i32_MAX	= 0x7FFFFFFF_i32;
constexpr i32 i32_MIN	= 0x80000000_i32;
constexpr c32 c32_MAX	= 0x7FFFFFFF_c32;
constexpr c32 c32_MIN	= 0x80000000_c32;
constexpr i64 i64_MAX	= 0x7FFFFFFFFFFFFFFF_i64;
constexpr i64 i64_MIN	= 0x8000000000000000_i64;

constexpr usz usz_MIN	= 0_usz;
constexpr usz usz_MAX	= usz(sizeof(usz) == 8 ? u64_MAX : u32_MAX);

// Helper for booleans on the GPU

class gbool {

public:

	inline gbool(bool b): val(b) {}
	inline operator bool() { return val != 0; }
	inline gbool &operator=(const gbool &other) = default;
	inline gbool(const gbool &other) = default;
	inline gbool &operator=(gbool &&other) = default;
	inline gbool(gbool &&other) = default;

	inline gbool &operator|=(gbool other) {
		val |= other.val;
		return *this;
	}

	inline gbool &operator&=(gbool other) {
		val &= other.val;
		return *this;
	}

	inline gbool &operator^=(gbool other) {
		val ^= other.val;
		return *this;
	}

	inline bool operator==(gbool other) const {
		return val == other.val;
	}

	inline bool operator!=(gbool other) const {
		return val != other.val;
	}

private:

	u8 val;
	u8 pad[3]{};

};

//Replacing branching by 2 MUL, 1 NOT, 1 AND

template<typename T>
static inline T pickIfTrue(T ifFalse, T ifTrue, bool val) {
	static_assert(std::is_arithmetic<T>::value, "Please use an arithmetic type in pickIfTrue");
	return ifFalse * (!val) + ifTrue * val;
}

template<typename T>
static inline T *pickIfTrue(T *ifFalse, T *ifTrue, bool val) {
	return (T*) pickIfTrue(usz(ifFalse), usz(ifTrue), val);
}

template<typename T>
static inline T pickIfTrue(T ifTrue, bool val) {
	static_assert(std::is_arithmetic<T>::value, "Please use an arithmetic type in pickIfTrue");
	return ifTrue * val;
}

template<typename T>
static inline T *pickIfTrue(T *ifTrue, bool val) {
	return (T*) pickIfTrue(usz(ifTrue), val);
}

//Helper function for properly destroying a pointer

template<typename T>
static inline void destroyPointer(T *&ptr) {
	if (ptr) {
		delete ptr;
		ptr = nullptr;
	}
}