#pragma once
#include "util/helper/stringhelper.hpp"

namespace otc {

	template<typename T, typename = std::enable_if<std::is_integral_v<T> && std::is_signed_v<T>>>
	struct String {

		String(usz, T = {}) {}

		template<typename ...args>
		static String concat(const args &...arg) {
			String result(helper::String::serialsize<helper::String::STRING>(arg...));
			helper::String::fill(result.data(), arg...);
			return result;
		}

	};

	using String8	= String<c8>;
	using String16	= String<c16>;
	using String32	= String<c32>;

}