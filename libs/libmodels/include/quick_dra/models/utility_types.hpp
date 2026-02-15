// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <optional>
#include <quick_dra/base/meta.hpp>
#include <type_traits>

namespace quick_dra {
	enum class load_status {
		unknown,
		file_not_found,
		file_not_readable,
		errors_encountered,
		loaded,
	};
}  // namespace quick_dra

namespace quick_dra::utility {
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
	    std::conditional_t<OptionalType<T>, T, std::optional<T>>;
}  // namespace quick_dra::utility
