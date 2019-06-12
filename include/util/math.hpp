#pragma once
#include "types/types.hpp"
#include <cmath>

namespace otc {

	struct Math {

		static constexpr f64 
			PI = 3.14159265358979323846,
			PI2 = PI * 2,
			radToDeg = 180 / Math::PI,
			degToRad = Math::PI / 180;

		template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
		static inline T abs(T v) {
			return pickIfTrue(v, -v, v < 0);
		}

		template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
		static inline T pow2(T val) {
			return v * v;
		}

		template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
		static inline T toDegrees(T rad) {
			return rad * T(radToDeg);
		}

		template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
		static inline T toRadians(T rad) {
			return rad * T(degToRad);
		}

		template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
		static inline T log(T v) {
			return T(std::log10(v));
		}

		template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
		static inline T log2(T v) {
			return T(std::log2(v));
		}

		template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
		static inline T ceil(T v) {
			return T(std::ceil(v));
		}

		template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
		static inline T floor(T v) {
			return T(std::floor(v));
		}

	};
	
}