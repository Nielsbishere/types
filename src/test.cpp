#include "types/string.hpp"
#include <stdio.h>
#include <vector>
#include <unordered_map>
#include <assert.h>
#include <iostream>

static auto splitString(const char *s) {
	return s;
}

HAS_FUNC_NAMED(hasBegin, begin);
HAS_FUNC_NAMED(hasEnd, end);
HAS_FUNC_NAMED(hasSize, size);
HAS_FUNC_NAMED_RET(hasSerializeEnd, serializeEnd, void);
HAS_FUNC_NAMED_RET(hasSerializeArray, serializeArray, void, const c8*);
HAS_FUNC_NAMED_RET(hasSerializeArrayEnd, serializeArrayEnd, void);
HAS_FUNC_NAMED_RET(hasSerializeObject, serializeObject, void, const c8*);
HAS_FUNC_NAMED_RET(hasSerializeObjectEnd, serializeObjectEnd, void);

template<typename T>
static constexpr bool isIterable = hasBegin<T> && hasEnd<T> && hasSize<T>;

template<template<typename> typename Serialize>
struct Serializer {

	//TODO: C-Style arrays
	//TODO: Serializer should be non-static, so it can have variables.
	//TODO: Cleanup! Array/Object funcs only need const u8*
	//TODO: Member variables
	//TODO: Allow objects as tuples

	template<typename T>
	static inline void serialize(const c8 *member, T &t) {

		//Arrays
		if constexpr (isIterable<T>) {

			member;

			if constexpr (hasSerializeArray<Serialize<T>>)
				Serialize<T>::serializeArray(member);

			for (auto it = t.begin(), end = t.end(); it != end; ++it) {

				serialize(nullptr, *it);

				if constexpr (hasSerializeEnd<Serialize<T>>) {

					auto next = it;
					++next;

					if(next != end)
						Serialize<T>::serializeEnd();
				}
			}

			if constexpr (hasSerializeArrayEnd<Serialize<T>>)
				Serialize<T>::serializeArrayEnd();

		}
		
		//Variables
		else if constexpr(std::is_arithmetic_v<T>)
			Serialize<T>::serialize(member, t);

		//Objects
		else {
			
			if constexpr (hasSerializeObject<Serialize<T>>)
				Serialize<T>::serializeObject(member);

			t.serialize<Serialize>(member);

			if constexpr (hasSerializeObjectEnd<Serialize<T>>)
				Serialize<T>::serializeObjectEnd();

		}
	}

	template<typename T, typename ...args>
	static inline void serialize(const c8 *member, T &t, args &...arg) {

		serialize(member, t);

		if constexpr (sizeof...(args) > 0) {

			if constexpr (hasSerializeEnd<Serialize<T>>)
				Serialize<T>::serializeEnd();

			serialize(member, arg...);
		}

	}

	template<typename T, typename ...args>
	static inline void serialize(T &t, args &...arg) {
		serialize(nullptr, t, arg...);
	}

};

struct Test {

	f32 x = f32(otc::Math::PI), y = 2.5f, z = 3, w = 1;
	std::vector<f32> h { 1, 2, 3 };

	template<template<typename> typename T>
	inline void serialize(const c8 *) {
		static const auto split = splitString("x, y, z, w, h");
		Serializer<T>::serialize/*Object*/(split, x, y, z, w, h);
	}

};

template<typename T/*, bool inObject */>
struct PrintSerializer {

	static inline void serialize(const c8 *, T &t) {
		std::cout << t;
	}

	static inline void serializeEnd() {
		printf(", ");
	}

	static inline void serializeArray(const c8 * /*, T &t */) {
		//if constexpr(inObject)
		//	printf("\"%s\": ", name);
		//else
		printf("[ ");
	}

	static inline void serializeArrayEnd() {
		printf(" ]");
	}

	static inline void serializeObject(const c8 *) {
		//if constexpr(inObject)
		//	printf("\"%s\": ", name);
		//else
		printf("{ ");
	}

	static inline void serializeObjectEnd() {
		printf(" }");
	}

};

void testSerialsize() {

	std::vector<Test> tests { {}, {} };
	Serializer<PrintSerializer>::serialize(tests);

	using arr = std::vector<f32>;
	using pair = std::pair<c8 *, f32>;
	using pairarr = std::vector<pair>;
	using umap = std::unordered_map<c8 *, f32>;

	arr vals { 1, 2, 3 };
	pair var { "abcd", 123 };
	pairarr vars { { "abcd", 123 }, { "efgh", 456 } };
	umap map = { { "abcd", 123 }, { "efgh", 456 } };

	//{ { 1, 2, 3 }, 5, 9, -3, 32.25, "Hello" } = 41
	usz cpp = otc::helper::String::serialsize<otc::helper::String::CPP>(vals, 5, 9, -3, 32.25, "Hello");
	assert(cpp == 41 && "Serialsize cpp returned invalid result");

	//{ 1, 2, 3 } 5 9 -3 32.25 Hello = 30
	usz string = otc::helper::String::serialsize<otc::helper::String::STRING>(vals, 5, 9, -3, 32.25, "Hello");
	assert(string == 30 && "Serialsize string returned invalid result");

	//{ 1, 2, 3 }59-332.25Hello = 25
	usz stringConcat = otc::helper::String::serialsize<otc::helper::String::CONCAT>(vals, 5, 9, -3, 32.25, "Hello");
	assert(stringConcat == 25 && "Serialsize concat returned invalid result");

	//[ { 1, 2, 3 }, 5, 9, -3, 32.25, "Hello" ] = 41
	usz json = otc::helper::String::serialsize<otc::helper::String::JSON>(vals, 5, 9, -3, 32.25, "Hello");
	assert(json == 41 && "Serialsize JSON returned invalid result");

	//{ 1, 2, 3 } = 11
	usz cppArr = otc::helper::String::serialsize<otc::helper::String::CPP>(vals);
	assert(cppArr == 11 && "Serialsize CPP array returned invalid result");

	//{ "abcd", 123 } = 15
	usz cppPair = otc::helper::String::serialsize<otc::helper::String::CPP>(var);
	assert(cppPair == 15 && "Serialsize cpp pair returned invalid result");

	//"abcd": 123 = 11
	usz jsonPair = otc::helper::String::serialsize<otc::helper::String::JSON>(var);
	assert(jsonPair == 11 && "Serialsize json pair returned invalid result");

	//{ { "abcd", 123 }, { "efgh", 456 } } = 36
	usz cppPairs = otc::helper::String::serialsize<otc::helper::String::CPP>(vars);
	assert(cppPairs == 36 && "Serialsize cpp pair array returned invalid result");

	//{ "abcd": 123, "efgh": 456 } = 28
	usz jsonPairs = otc::helper::String::serialsize<otc::helper::String::JSON>(vars);
	assert(jsonPairs == 28 && "Serialsize json pair array returned invalid result");

	//{ { "abcd", 123 }, { "efgh", 456 } } = 36
	usz cppMap = otc::helper::String::serialsize<otc::helper::String::CPP>(map);
	assert(cppMap == 36 && "Serialsize cpp map returned invalid result");

	//{ "abcd": 123, "efgh": 456 } = 28
	usz jsonMap = otc::helper::String::serialsize<otc::helper::String::JSON>(map);
	assert(jsonMap == 28 && "Serialsize json map returned invalid result");

	printf("Serialsize checks passed\n");

}

int main(){
	testSerialsize();
	printf("All checks passed\n");
	system("pause");
	return 0;
}