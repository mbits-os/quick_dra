// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <map>
#include <optional>
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

	template <typename T>
	struct is_optional : std::false_type {};

	template <typename T>
	struct is_optional<T&> : is_optional<T> {};

	template <typename T>
	struct is_optional<T const> : is_optional<T> {};

	template <typename T>
	struct is_optional<std::optional<T>> : std::true_type {};

	template <typename T>
	concept OptionalType = static_cast<bool>(is_optional<T>{});

	template <typename T>
	concept RequiredType = !OptionalType<T>;

	template <typename T, typename V>
	concept optional_of =
	    OptionalType<T> &&
	    std::same_as<V, typename std::remove_cvref_t<T>::value_type>;
}  // namespace quick_dra
