#include "types/string.hpp"
#include <stdio.h>
#include <vector>
#include <unordered_map>
#include <assert.h>
#include <iostream>
#include <array>

HAS_FUNC_NAMED(hasBegin, begin);
HAS_FUNC_NAMED(hasEnd, end);
HAS_FUNC_NAMED(hasSize, size);
HAS_FUNC_NAMED_RET(hasSerializeEnd, serializeEnd, void);
HAS_FUNC_NAMED_RET(hasSerializeArrayEnd, serializeArrayEnd, void);
HAS_T_FUNC_NAMED(hasSerialize, serialize);
HAS_T_FUNC_NAMED(hasSerializeTuple, serializeTuple);

template<typename T>
static constexpr bool isIterable = hasBegin<T> && hasEnd<T> && hasSize<T>;

template<typename Serialize>
struct Serializer {

	//TODO: C-Style arrays
	//TODO: Strings

	template<bool inObject, typename T>
	static inline void serialize(Serialize &serializer, const c8 *member, T &t) {

		static constexpr bool
			hasSerialize_ = hasSerialize<T, Serialize, Serialize &, const c8*>,
			hasSerializeTuple_ = hasSerializeTuple<T, Serialize, Serialize &, const c8*>;

		//Arrays
		if constexpr (isIterable<T>) {

			//member;

			//TODO: This isn't safe!
			serializer.serializeArray<inObject>(member, t);

			for (auto it = t.begin(), end = t.end(); it != end; ++it) {

				serialize<false>(serializer, nullptr, *it);

				if constexpr (hasSerializeEnd<Serialize>) {

					auto next = it;
					++next;

					if (next != end)
						serializer.serializeEnd();
				}
			}

			if constexpr (hasSerializeArrayEnd<Serialize>)
				serializer.serializeArrayEnd();

		}

		//Variables
		else if constexpr (std::is_arithmetic_v<T>) {
			//TODO: This isn't safe!
			serializer.serialize<inObject>(member, t);
		}

		//Objects
		else if constexpr (hasSerialize_ || hasSerializeTuple_) {

			//TODO: This isn't safe!
			serializer.serializeObject<inObject, hasSerializeTuple_>(member);

			if constexpr (hasSerialize_)
				t.serialize<Serialize>(serializer, member);
			else
				t.serializeTuple<Serialize>(serializer, member);

			//TODO: This isn't safe!
			serializer.serializeObjectEnd<hasSerializeTuple_>();

		}

		else
			static_assert(false, "The object can't be serialized; it should be a container, data type or have serialization functions!");
	}

	template<bool inObject, typename T, typename ...args>
	static inline void serialize(Serialize &serializer, const c8 *member, T &t, args &...arg) {

		serialize<inObject>(serializer, member, t);

		if constexpr (sizeof...(args) > 0) {

			if constexpr (hasSerializeEnd<Serialize>)
				serializer.serializeEnd();

			serialize<inObject>(serializer, member, arg...);
		}

	}

	template<usz offset = 0, usz memberCount, typename T, typename ...args>
	static inline void serializeObject(Serialize &serializer, const std::array<std::string, memberCount> &members, const c8 *member, T &t, args &...arg){

		member;
		serialize<true>(serializer, members[offset].data(), t);

		if constexpr (sizeof...(args) > 0) {

			if constexpr (hasSerializeEnd<Serialize>)
				serializer.serializeEnd();

			serializeObject<offset + 1>(serializer, members, member, arg...);
		}

	}

	template<typename T, typename ...args>
	static inline Serialize serialize(T &t, args &...arg) {

		Serialize serializer;

		//TODO: This isn't safe!
		serializer.serializeObject<false, true>(nullptr);

		serialize<false>(serializer, nullptr, t, arg...);

		//TODO: This isn't safe!
		serializer.serializeObjectEnd<true>();

		return serializer;
	}

};

#define otc_serialize_tuple(version, ...)											\
static constexpr u64 struct_version = version;										\
template<typename T>																\
inline void serializeTuple(T &serializer, const c8 *member) {						\
	Serializer<T>::serialize<false>(serializer, member, __VA_ARGS__);				\
}

#define otc_serialize(version, ...)													\
static constexpr u64 struct_version = version;										\
template<typename T>																\
inline void serialize(T &serializer, const c8 *member) {							\
	static const auto members = otc::getMemberNames(#__VA_ARGS__, __VA_ARGS__);		\
	Serializer<T>::serializeObject(serializer, members, member, __VA_ARGS__);		\
}

struct PrintSerializer {

	template<bool inObject, typename T>
	inline void serialize(const c8 *member, T &t) {

		member;

		if constexpr (inObject) 
			printf("\"%s\": ", member);

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

	otc_serialize(0, x, y, z, w, h);
};

struct Test2 {

	Test x{}, y{};
	std::vector<Test> z { {}, {} };
	Vector2 test;

	otc_serialize(0, x, y, z, test);
};

void testSerialsize() {

	Test2 tests;
	f32 value{};
	Serializer<PrintSerializer>::serialize(tests, value);

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