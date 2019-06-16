#include "types/string.hpp"
#include "util/serialize.hpp"
#include "util/math.hpp"
#include <stdio.h>
#include <unordered_map>
#include <assert.h>
#include <iostream>


struct JSONSerialsize {

	usz size = 0;

	inline usz escapeChar(const c8 c) {
		if (c == '\n' || c == '\t' || c == '\r' || c == '\f' || c == '\b' || c == '\\' || c == '\"')
			return 2;			//\x
		else if (u8(c) < 0x20)
			return 6;			//\uxxxx
		return 1;
	}

	template<typename T>
	inline usz escapeString(const T beg, const T end) {

		usz siz = 0;

		for (auto it = beg; it != end; ++it)
			siz += escapeChar(*it);

		return siz;
	}

	template<bool inObject, usz count, typename T>
	inline void serialize(const c8 *member, T &t) {

		member;

		if constexpr (inObject)
			size += strlen(member) + 3;

		//Char
		if constexpr (std::is_same_v<T, c8> || std::is_same_v<T, const c8>)
			size += escapeChar(t) + 2;

		//String
		else if constexpr (otc::util::isIterable<T>)
			size += escapeString(t.begin(), t.end()) + 2;

		else if constexpr (std::is_same_v<T, const c8*> || std::is_same_v<T, c8*> || std::is_same_v<T, c8* const> || std::is_same_v<T, const c8* const>){
			if constexpr(count == 0)
				size += escapeString((c8*)t, (c8*)t + strlen(t)) + 2;
			else
				size += escapeString((c8*)t, (c8*)t + count) + 2;
		}

		//Bool
		else if constexpr (std::is_same_v<T, bool>)
			size += 4 + usz(!t);

		//Number
		else if constexpr (std::is_floating_point_v<T>)
			size += snprintf(nullptr, 0, "%.15g", t);
		else if constexpr(std::is_unsigned_v<T>)
			size += snprintf(nullptr, 0, "%llu", t);
		else if constexpr (std::is_integral_v<T>)
			size += snprintf(nullptr, 0, "%lld", t);

		//Undefined
		else
			static_assert(false, "Couldn't serialize");
	}

	inline void serializeEnd() { ++size; }

	template<bool inObject, typename T>
	inline void serializeArray(const c8 *member, T &) {

		member;

		if constexpr (inObject)
			size += strlen(member) + 3;

		++size;
	}

	inline void serializeArrayEnd() { ++size; }

	template<bool inObject, bool isTuple>
	inline void serializeObject(const c8 *member) {

		member;

		if constexpr (inObject)
			size += strlen(member) + 3;

		++size;
	}

	template<bool isTuple>
	inline void serializeObjectEnd() { ++size; }

};

struct JSONWriter {

	using Serialsize = JSONSerialsize;

	std::string result;
	usz offset{}, size;

	static constexpr std::pair<c8, c8> escaped[] = { 
		{ '\n', 'n' }, { '\t', 't' }, { '\r', 'r' },
		{ '\f', 'f' }, { '\b', 'b' }, { '\\', '\\' }, { '\"', '\"' }
	};

	JSONWriter(JSONSerialsize &totalSize): size(totalSize.size), result(totalSize.size, '0') { }

	inline void escapeCharacter(const c8 c) {

		for(auto &esc : escaped)
			if (esc.first == c) {
				result[offset] = '\\';
				result[offset + 1] = esc.second;
				offset += 2;
				return;
			}

		if (u8(c) < 0x20) {

			u8 lo = u8(c & 0xF);
			u8 hi = u8(c & 0xF0) >> 4;

			result[offset] = '\\';
			result[offset + 1] = 'u';
			result[offset + 4] = hi < 0xA ? '0' + hi : hi + ('A' - 0xA);
			result[offset + 5] = lo < 0xA ? '0' + lo : lo + ('A' - 0xA);
			offset += 6;
			return;
		}

		result[offset] = c;
		++offset;
	}

	template<bool inObject, usz count, typename T>
	inline void serialize(const c8 *member, T &t) {

		member;

		if constexpr (inObject)
			offset += snprintf(result.data() + offset, size - offset, "\"%s\":", member);

		if constexpr (std::is_same_v<T, c8> || std::is_same_v<T, const c8>) {		//Characters by themselves aren't supported, so they are size=1 strings

			result[offset] = '\"';
			++offset;

			escapeCharacter(t);

			result[offset] = '\"';
			++offset;
		} 
		
		else if constexpr (otc::util::isIterable<T> || std::is_same_v<T, const c8 *> || std::is_same_v<T, c8 *> || std::is_same_v<T, c8 * const> || std::is_same_v<T, const c8 * const>) {

			result[offset] = '"';
			++offset;

			if constexpr(otc::util::isIterable<T>)
				for(auto it = t.begin(), end = t.end(); it != end; ++it)
					escapeCharacter(*it);
			else if constexpr(count == 0)
				for(c8 *it = (c8*)t, *end = it + strlen(t); it != end; ++it)
					escapeCharacter(*it);
			else
				for (c8 *it = ( c8 *) t, *end = it + count; it != end; ++it)
					escapeCharacter(*it);

			result[offset] = '"';
			++offset;
		}

		//Bool
		else if constexpr (std::is_same_v<T, bool>){

			if(!t)
				memcpy(result.data() + offset, "false", 5);
			else
				memcpy(result.data() + offset, "true", 4);

			offset += 4 + usz(!t);
		}

		//Number
		else if constexpr (std::is_floating_point_v<T>)
			offset += snprintf(result.data() + offset, size - offset, "%.15g", t);
		else if constexpr (std::is_unsigned_v<T>)
			offset += snprintf(result.data() + offset, size - offset, "%llu", t);
		else if constexpr (std::is_integral_v<T>)
			offset += snprintf(result.data() + offset, size - offset, "%lld", t);

		//Undefined
		else 
			static_assert(false, "Couldn't serialize");
	}

	inline void serializeEnd() {
		result[offset] = ',';
		++offset;
	}

	template<bool inObject, typename T>
	inline void serializeArray(const c8 *member, T &) {

		member;

		if constexpr (inObject)
			offset += snprintf(result.data() + offset, size - offset, "\"%s\":", member);

		result[offset] = '[';
		++offset;
	}

	inline void serializeArrayEnd() {
		result[offset] = ']';
		++offset;
	}

	template<bool inObject, bool isTuple>
	inline void serializeObject(const c8 *member) {

		member;

		if constexpr (inObject) 
			offset += snprintf(result.data() + offset, size - offset, "\"%s\":", member);

		if constexpr (isTuple)
			result[offset] = '[';
		else
			result[offset] = '{';

		++offset;
	}

	template<bool isTuple>
	inline void serializeObjectEnd() {

		if constexpr (isTuple)
			result[offset] = ']';
		else
			result[offset] = '}';

		++offset;
	}

};

struct Vector2 {

	f32 x, y;

	Vector2(f32 x = 4, f32 y = 5): x(x), y(y) {}

	otc_serialize_tuple(0, x, y);
};

struct Test {

	f32 x = f32(otc::Math::PI), y = 2.5f, z = 3, w = 1;
	std::vector<f32> h { 1, 2, 3 };
	std::vector<std::string> strings { "dfaisdfj", "dfioasdf" };

	otc_serialize(0, x, y, z, w, h, strings);
};

struct Test2 {

	Test x{}, y{};
	std::vector<Test> z { {}, {} };
	Vector2 test;
	std::string myString = "haha\nMyDude";
	c8 myChars[47] = "Checkmate\n---\"-\0--\teojiasdf\rdisof\f\xA0\x80\x00ifjaodsfi";
	c8 character = '"';
	bool testing = true, testing0 = false;

	otc_serialize(0, x, y, z, test, myString, myChars, character, testing, testing0);
};

void testSerialsize() {

	using arr = std::vector<f32>;
	using pair = std::pair<c8*, f32>;
	using pairarr = std::vector<pair>;
	using umap = std::unordered_map<c8*, f32>;

	arr vals { 1, 2, 3 };
	pair var { "abcd", 123 };
	pairarr vars { { "abcd", 123 }, { "efgh", 456 } };
	umap map = { { "abcd", 123 }, { "efgh", 456 } };

	Test2 tests;
	f32 value {};

	JSONWriter jsonSerialize = otc::Serializer::serialize<JSONWriter>(tests, value, map, vars, var, vals);

	printf("%s\n%llu\n", jsonSerialize.result.data(), jsonSerialize.size);

	printf("Serialsize checks passed\n");

}

int main(){
	testSerialsize();
	printf("All checks passed\n");
	system("pause");
	return 0;
}