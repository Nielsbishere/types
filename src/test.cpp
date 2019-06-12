#include "types/string.hpp"
#include "util/serialize.hpp"
#include <stdio.h>
#include <unordered_map>
#include <assert.h>
#include <iostream>

struct PrintSerializer {

	template<bool inObject, typename T>
	inline void serialize(const c8 *member, T &t) {

		member;

		if constexpr (inObject) 
			printf("\"%s\": ", member);

		if constexpr(std::is_same_v<T, c8> || std::is_same_v<T, const c8>)
			std::cout << '\'' << t << '\'';
		else if constexpr (otc::util::isIterable<T>)
			std::cout << '\"' << t << '\"';
		else
			std::cout << t;
	}

	inline void serializeEnd() {
		printf(", ");
	}

	template<bool inObject, typename T>
	inline void serializeArray(const c8 *member, T &) {

		member;

		if constexpr (inObject)
			printf("\"%s\": ", member);

		printf("[ ");
	}

	inline void serializeArrayEnd() {
		printf(" ]");
	}

	template<bool inObject, bool isTuple>
	inline void serializeObject(const c8 *member) {

		member;

		if constexpr (inObject)
			printf("\"%s\": ", member);

		if constexpr (isTuple)
			printf("[ ");
		else
			printf("{ ");
	}

	template<bool isTuple>
	inline void serializeObjectEnd() {
		if constexpr (isTuple)
			printf(" ]");
		else
			printf(" }");
	}

};

struct Vector2 {

	f32 x, y;
	//f32 zw[2];

	Vector2(f32 x = 4, f32 y = 5): x(x), y(y)/*, zw { 7, 9.333 } */ {}

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
	std::string myString = "haha";

	otc_serialize(0, x, y, z, test, myString);
};

void testSerialsize() {

	Test2 tests;
	f32 value{};
	otc::Serializer<PrintSerializer>::serialize(tests, value);

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