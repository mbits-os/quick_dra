// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <variant>
#include <vector>

namespace quick_dra {
	template <typename T>
	using maybe_list = std::variant<T, std::vector<T>>;

	template <typename T>
	inline bool maybe_is_scalar(maybe_list<T> const& maybe) {
		return std::holds_alternative<T>(maybe);
	}

	template <typename T>
	inline bool maybe_is_array(maybe_list<T> const& maybe) {
		return std::holds_alternative<std::vector<T>>(maybe);
	}

	template <typename T>
	inline T& maybe_scalar(maybe_list<T>& maybe) {
		return std::get<T>(maybe);
	}

	template <typename T>
	inline T const& maybe_scalar(maybe_list<T> const& maybe) {
		return std::get<T>(maybe);
	}

	template <typename T>
	inline std::vector<T>& maybe_array(maybe_list<T>& maybe) {
		return std::get<std::vector<T>>(maybe);
	}

	template <typename T>
	inline std::vector<T> const& maybe_array(maybe_list<T> const& maybe) {
		return std::get<std::vector<T>>(maybe);
	}

	template <typename T>
	using mapped_value = std::map<unsigned, maybe_list<T>>;

	template <typename T, typename... Args>
	struct expand_args;

	template <typename T, typename... Args>
	using expand_args_t = typename expand_args<T, Args...>::type;

	template <template <typename...> class C, typename... T, typename... Args>
	struct expand_args<C<T...>, Args...> {
		using type = C<T..., Args...>;
	};
}  // namespace quick_dra
