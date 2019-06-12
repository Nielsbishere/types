#pragma once
#include "util/math.hpp"
#include "util/template.hpp"
#include <cstring>

namespace otc {

	namespace helper {

		HAS_FUNC_NAMED(hasBegin, begin);
		HAS_FUNC_NAMED(hasEnd, end);
		HAS_FIELD_NAMED(hasFirst, first);
		HAS_FIELD_NAMED(hasSecond, second);

		struct String {

			enum SerialType {
				CPP,		//{ { 1, 2, 3, 4 }, 1, 2, 3, 4, -1.3, { { "abc", 3 }, { "cdef", 5 } } }
				STRING,		//{ 1, 2, 3, 4 } 1 2 3 4 -1.3 { { "abc", 3 }, { "cdef", 5 } }
				JSON,		//[ [ 1, 2, 3, 4 ], 1, 2, 3, 4, -1.3, [ "abc": 3, "cdef": 5 ] ]
				CONCAT		//{ 1, 2, 3, 4 }1234-1.3{ { "abc", 3 }, { "cdef", 5 } }
			};

			template<SerialType type>
			static constexpr bool useLowestLevelCommas = type != SerialType::STRING && type != SerialType::CONCAT;		//1 2 3 instead of 1, 2, 3

			template<SerialType type>
			static constexpr bool useLowestLevelSpaces = type != SerialType::CONCAT;			//1,2,3 instead of 1, 2, 3

			template<SerialType type>
			static constexpr bool prefixObjectByType = type == SerialType::CPP;					//MyStruct{ 1, 2, 3 } instead of { 1, 2, 3 }

			template<SerialType type>
			static constexpr bool useKeyPairNotation = type == SerialType::JSON;				//"myVal": 1 instead of { "myVal", 1 }

			template<SerialType type>
			static constexpr bool surroundInBrackets = type != SerialType::STRING && type != SerialType::CONCAT;	//{ 1, 2, 3 } instead of 1, 2, 3

			template<SerialType type>
			static constexpr bool useStringQuotes = type != SerialType::STRING && type != SerialType::CONCAT;		//"hello" instead of hello

			/*template<SerialType type>
			static constexpr c16 arrayBrackets[2] = type == SerialType::JSON ? L"[]" : L'{}';*/

			template<SerialType type, typename T>
			static inline usz serialsize_(const T &v) {

				//floats/ints/uints
				if constexpr (std::is_arithmetic_v<T>) {

					if (v == 0 || v == 1)
						return 1;

					if constexpr (std::is_signed_v<T>) {
						if (v == -1)
							return 2;
					}

					if constexpr (std::is_floating_point_v<T>)
						return snprintf(nullptr, 0, "%.15g", v);
					else if constexpr (std::is_unsigned_v<T>)
						return usz(Math::ceil(Math::log(f64(v))));
					else
						return usz(Math::ceil(Math::log(Math::abs(f64(v))))) + usz(v < 0);
				}
				
				//"xyz" (TODO: Escaping characters)
				else if constexpr (std::is_same_v<T, c8*>)
					return strlen(v) + (useStringQuotes<type> ? 2 : 0);

				//[ x, y, z ] = { x, y, z }
				else if constexpr (hasBegin<T> && hasEnd<T>) {

					usz count = 0, n = 0;

					for (auto beg = v.begin(); beg != v.end(); ++beg, ++n)
						count += serialsize_<type>(*beg);

					return count + 3 + pickIfTrue(n + n - 1, n > 0);
				}

				//{ x, y } = x: y (TODO: JSON edge case; if not in object or pair[])
				else if constexpr (hasFirst<T> && hasSecond<T>) {

					if constexpr (useKeyPairNotation<type> && (otc::util::canCastSafely<const c8*, decltype(v.first)>))
						return 2 + serialsize_<type>(v.first) + serialsize_<type>(v.second);
					else
						return 6 + serialsize_<type>(v.first) + serialsize_<type>(v.second);

				}
				
				//TODO: Objects
				else 
					return 0;
			}

			//"xyz"	(TODO: Escaping characters)
			//[ -3, -5, -9 ] = { -3, -5, -9 }
			template<SerialType type, typename T, usz n>
			static inline usz serialsize_(const T (&v)[n]) {

				if constexpr (std::is_same_v<T, c8> || std::is_same_v<T, c16> || std::is_same_v<T, c32>)
					return (v[n - 1] == 0 ? n - 1 : n) + (useStringQuotes<type> ? 2 : 0);
				else {

					usz res = 0;

					for (usz i = 0; i < n; ++i)
						res += serialsize_<type>(v[i]);

					return res + 3 + pickIfTrue(n + n - 1, n > 0);
				}
			}

			//Serialize object internal
			//[ *, *, * ] or { *, *, * } or * * *
			template<SerialType type, typename T, typename ...args>
			static inline usz serialsize(const T &t, const args &...arg) {

				constexpr usz argc = sizeof...(arg);

				usz len = String::serialsize_<type>(t);
				const auto sizes = { (String::serialsize_<type>(arg))... };

				for (const usz u : sizes)
					len += u;

				len += useLowestLevelCommas<type> ? argc : 0;
				len += useLowestLevelSpaces<type> ? argc : 0;
				return surroundInBrackets<type> ? len + 3 + usz(len != 0) : len;
			}

			//Serialize single object
			template<SerialType type, typename T>
			static inline usz serialsize(const T &t) {
				return String::serialsize_<type>(t);
			}

		};

	}

}