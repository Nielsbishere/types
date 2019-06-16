#pragma once
#include "template.hpp"
#include "types/types.hpp"
#include <array>
#include <string>

namespace otc {

	namespace util {

		///Helper for iterables

		HAS_FUNC_NAMED(hasBegin, begin);
		HAS_FUNC_NAMED(hasEnd, end);
		HAS_FUNC_NAMED(hasSize, size);
		HAS_FIELD_NAMED(hasFirst, first);
		HAS_FIELD_NAMED(hasSecond, second);

		template<typename T>
		static constexpr bool isIterable = hasBegin<T> && hasEnd<T> && hasSize<T>;

		///Helper for Serializer classes

		HAS_FUNC_NAMED_RET(hasSerializeEnd, serializeEnd, void);
		HAS_FUNC_NAMED_RET(hasSerializeArrayEnd, serializeArrayEnd, void);
		
			///Serialize array check

			namespace templates {
			
				template<typename T, typename T2, bool b, typename = int>
				struct HasSerializeArray : std::false_type {};
			
				template<typename T, typename T2, bool b>
				struct HasSerializeArray<T, T2, b, decltype(std::declval<T>().template serializeArray<b, T2>(std::declval<const c8*>(), std::declval<T2&>()), 0)> : std::true_type {};
			
			}

			template<typename T, typename T2, bool b>
			static constexpr bool hasSerializeArray = templates::HasSerializeArray<T, T2, b, int>::value;
		
			///Serialize object check

			namespace templates {
			
				template<typename T, bool b0, bool b1, typename = int>
				struct HasSerializeObject : std::false_type {};
			
				template<typename T, bool b0, bool b1>
				struct HasSerializeObject<T, b0, b1, decltype(std::declval<T>().template serializeObject<b0, b1>(std::declval<const c8*>()), 0)> : std::true_type {};
			
			}

			template<typename T, bool b0, bool b1>
			static constexpr bool hasSerializeObject = templates::HasSerializeObject<T, b0, b1, int>::value;
		
			///Serialize object end check

			namespace templates {
			
				template<typename T, bool b, typename = int>
				struct HasSerializeObjectEnd : std::false_type {};
			
				template<typename T, bool b>
				struct HasSerializeObjectEnd<T, b, decltype(std::declval<T>().template serializeObjectEnd<b>(), 0)> : std::true_type {};
			
			}

			template<typename T, bool b>
			static constexpr bool hasSerializeObjectEnd = templates::HasSerializeObjectEnd<T, b, int>::value;
		
			///Serialize check

			namespace templates {
			
				template<typename T, bool b, usz count, typename T2, typename = int>
				struct HasSerializer : std::false_type {};
			
				template<typename T, bool b, usz count, typename T2>
				struct HasSerializer<T, b, count, T2, 
					decltype(std::declval<T>().template serialize<b, count, T>(std::declval<const c8*>(), std::declval<T&>()), 0)
				> : std::true_type {};
			
			}

			template<typename T, bool b, usz count, typename T2>
			static constexpr bool hasSerializer = templates::HasSerializer<T, b, count, T2, int>::value;

		///Helper for serializable objects

		HAS_T_FUNC_NAMED(hasSerialize, serialize);
		HAS_T_FUNC_NAMED(hasSerializeTuple, serializeTuple);

		///Parsing member functions from macro

		template<typename ...args>
		static auto getMemberNames(const c8 *s, const args &...) {

			std::array<std::string, sizeof...(args)> strings {};

			const usz size = strlen(s);
			bool hasStarted = false;

			for (usz i = 0, elements = 0, prev = 0; i < size; ++i) {

				const c8 &c = s[i];

				if (c == ',' && hasStarted) {
					strings[elements] = std::string(s + prev, s + i);
					hasStarted = false;
					++elements;
				} else if (c != ' ' && c != '\t' && !hasStarted) {

					prev = i;
					hasStarted = true;
				}

				if (i == size - 1)
					strings[elements] = std::string(s + prev, s + i + 1);

			}


			return strings;
		}

	}

	template<typename Serialize>
	struct TSerializer {

		template<bool inObject, typename T>
		static inline void serialize(Serialize &serializer, const c8 *member, T &t) {

			constexpr bool
				hasSerialize_ = util::hasSerialize<T, Serialize, Serialize&, const c8*>,
				hasSerializeTuple_ = util::hasSerializeTuple<T, Serialize, Serialize&, const c8*>;

			member;

			//Arrays
			if constexpr (util::isIterable<T>) {

				using Iterated = decltype(*t.begin());

				if constexpr (std::is_same_v<Iterated, c8&> || std::is_same_v<Iterated, const c8&>) {
					if constexpr(util::hasSerializer<Serialize, inObject, 0, T>)
						serializer.template serialize<inObject, 0>(member, t);
				}

				else {

					if constexpr(util::hasSerializeArray<Serialize, T, inObject>)
						serializer.serializeArray<inObject>(member, t);

					for (auto it = t.begin(), end = t.end(); it != end; ++it) {

						serialize<false>(serializer, nullptr, *it);

						if constexpr (util::hasSerializeEnd<Serialize>) {

							auto next = it;
							++next;

							if (next != end)
								serializer.serializeEnd();
						}
					}

					if constexpr (util::hasSerializeArrayEnd<Serialize>)
						serializer.serializeArrayEnd();

				}
			}

			//Variables
			else if constexpr (std::is_arithmetic_v<T>) {
				if constexpr (util::hasSerializer<Serialize, inObject, 0, T>)
					serializer.template serialize<inObject, 0>(member, t);
			}

			//Objects
			else if constexpr (hasSerialize_ || hasSerializeTuple_) {

				if constexpr (util::hasSerializeObject<Serialize, inObject, hasSerializeTuple_>)
					serializer.template serializeObject<inObject, hasSerializeTuple_>(member);

				if constexpr (hasSerialize_)
					t.template serialize<Serialize>(serializer, member);
				else
					t.template serializeTuple<Serialize>(serializer, member);

				if constexpr (util::hasSerializeObjectEnd<Serialize, hasSerializeTuple_>)
					serializer.template serializeObjectEnd<hasSerializeTuple_>();

			}

			//Pairs
			else if constexpr (util::hasFirst<T> && util::hasSecond<T>) {

				if constexpr (util::hasSerializeObject<Serialize, inObject, true>)
					serializer.template serializeObject<inObject, true>(member);

				serialize<false>(serializer, nullptr, t.first);

				if constexpr (util::hasSerializeEnd<Serialize>)
					serializer.serializeEnd();

				serialize<false>(serializer, nullptr, t.second);

				if constexpr (util::hasSerializeObjectEnd<Serialize, true>)
					serializer.template serializeObjectEnd<true>();

			}

			//String
			else if constexpr (std::is_same_v<T, const c8*> || std::is_same_v<T, c8*> || std::is_same_v<T, const c8* const> || std::is_same_v<T, c8* const>) {
				if constexpr (util::hasSerializer<Serialize, inObject, 0, T>) {
					c8 *v = (c8*)t;
					serializer.template serialize<inObject, 0>(member, v);
				}
			}

			//Undefined
			else
				static_assert(false, "The object can't be serialized; it should be a container, data type or have serialization functions!");
		}

		template<bool inObject, typename T, usz N>
		static inline void serialize(Serialize &serializer, const c8 *member, T (&t)[N]) {

			if constexpr (std::is_same_v<T, const c8> || std::is_same_v<T, c8>) {
				if constexpr (util::hasSerializer<Serialize, inObject, N, const c8*>) {
					c8 *str = t;
					serializer.template serialize<inObject, N>(member, str);
				}
			} else {

				if constexpr (util::hasSerializeObject<Serialize, inObject, true>)
					serializer.template serializeObject<inObject, true>(member);

				member;

				for (T *i = t, *end = i + N, *last = end - 1; i < end; ++i) {

					serialize<false>(serializer, nullptr, *i);

					if constexpr (util::hasSerializeEnd<Serialize>) {
						if (i != last)
							serializer.serializeEnd();
					}

				}

				if constexpr (util::hasSerializeObjectEnd<Serialize, true>)
					serializer.template serializeObjectEnd<true>();

			}
		}

		template<bool inObject, typename T, typename ...args>
		static inline void serialize(Serialize &serializer, const c8 *member, T &t, args &...arg) {

			serialize<inObject>(serializer, member, t);

			if constexpr (sizeof...(args) > 0) {

				if constexpr (util::hasSerializeEnd<Serialize>)
					serializer.serializeEnd();

				serialize<inObject>(serializer, member, arg...);
			}

		}

		template<usz offset = 0, usz memberCount, typename T, typename ...args>
		static inline void serializeObject(Serialize &serializer, const std::array<std::string, memberCount> &members, const c8 *member, T &t, args &...arg) {

			member;
			serialize<true>(serializer, members[offset].data(), t);

			if constexpr (sizeof...(args) > 0) {

				if constexpr (util::hasSerializeEnd<Serialize>)
					serializer.serializeEnd();

				serializeObject<offset + 1>(serializer, members, member, arg...);
			}

		}

		template<typename T, typename ...args>
		static inline void serialize(Serialize &serializer, T &t, args &...arg) {

			if constexpr (util::hasSerializeObject<Serialize, false, true>)
				serializer.template serializeObject<false, true>(nullptr);

			serialize<false>(serializer, nullptr, t, arg...);

			if constexpr (util::hasSerializeObjectEnd<Serialize, true>)
				serializer.template serializeObjectEnd<true>();
		}

	};

	struct Serializer {

		template<typename Serialize, typename T2, typename ...args>
		static inline Serialize serialize(T2 &t, args &...arg) {

			using Serialsize = typename Serialize::Serialsize;

			Serialsize size;
			TSerializer<Serialsize>::serialize(size, t, arg...);

			Serialize serial(size);
			TSerializer<Serialize>::serialize(serial, t, arg...);
			return serial;
		}

	};

}

#define otc_serialize_tuple(version, ...)													\
static constexpr u64 struct_version = version;												\
template<typename T>																		\
inline void serializeTuple(T &serializer, const c8 *member) {								\
	otc::TSerializer<T>::serialize<false>(serializer, member, __VA_ARGS__);					\
}

#define otc_serialize(version, ...)															\
static constexpr u64 struct_version = version;												\
template<typename T>																		\
inline void serialize(T &serializer, const c8 *member) {									\
	static const auto members = otc::util::getMemberNames(#__VA_ARGS__, __VA_ARGS__);		\
	otc::TSerializer<T>::serializeObject(serializer, members, member, __VA_ARGS__);			\
}