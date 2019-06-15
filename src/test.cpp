#include "types/string.hpp"
#include "util/serialize.hpp"
#include "util/math.hpp"
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
		else if constexpr (otc::util::isIterable<T> || std::is_same_v<T, const c8*> || std::is_same_v<T, c8*>)
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
	std::string myString = "haha";
	const c8 myChars[19] = "Checkmate atheists";

	otc_serialize(0, x, y, z, test, myString, myChars);
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
	otc::Serializer<PrintSerializer>::serialize(tests, value, map, vars, var, vals);

	printf("Serialsize checks passed\n");

}

int main(){
	testSerialsize();
	printf("All checks passed\n");
	system("pause");
	return 0;
}