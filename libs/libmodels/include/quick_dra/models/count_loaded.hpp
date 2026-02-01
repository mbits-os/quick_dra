// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <optional>
#include <quick_dra/models/utility_types.hpp>
#include <utility>

namespace quick_dra::v1::partial {
	using namespace utility;

	template <required_type T, typename Q>
	std::pair<int, int> check_if_loaded(std::optional<Q> const& src) noexcept {
		return {1, src ? 1 : 0};
	}

	template <optional_type T, typename Q>
	std::pair<int, int> check_if_loaded(std::optional<Q> const& src) noexcept {
		return {0, src ? 1 : 0};
	}

	template <std::same_as<std::pair<int, int>>... T>
	std::pair<int, int> sum_loaded(T const&... pairs) noexcept {
		return {
		    (std::get<0>(pairs) + ... + 0),
		    (std::get<1>(pairs) + ... + 0),
		};
	}
}  // namespace quick_dra::v1::partial
