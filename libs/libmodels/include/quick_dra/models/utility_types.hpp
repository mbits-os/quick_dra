// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <optional>
#include <type_traits>

namespace quick_dra {
	enum class load_status {
		unknown,
		file_not_found,
		file_not_readable,
		errors_encountered,
		empty,
		partially_loaded,
		fully_loaded,
	};
}  // namespace quick_dra

namespace quick_dra::utility {
	template <typename T>
	struct is_optional : std::false_type {};

	template <typename T>
	struct is_optional<std::optional<T>> : std::true_type {};

	template <typename T>
	concept optional_type = static_cast<bool>(is_optional<T>{});

	template <typename T>
	concept required_type = !optional_type<T>;

	template <typename S, typename D>
	concept partial_type_with_copy_to = requires(D& dst, S const& src) {
		{ src.copy_to(dst) } -> std::same_as<bool>;
	};  // NOLINT(readability/braces)

	template <typename S, typename D>
	concept partial_type_with_merge = requires(D& dst, S const& src) {
		{ src.merge_into(dst) } -> std::same_as<bool>;
	};  // NOLINT(readability/braces)

	template <typename T>
	using add_optional =
	    std::conditional_t<optional_type<T>, T, std::optional<T>>;
}  // namespace quick_dra::utility
