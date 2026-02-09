// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <functional>
#include <quick_dra/models/types.hpp>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace quick_dra {
	std::vector<unsigned> search_insured_from_position(
	    unsigned position,
	    std::span<partial::insured_t> const& insured,
	    std::function<void(std::string const&)> const& on_error);

	std::vector<unsigned> search_insured_from_keyword(
	    std::string_view search_keyword,
	    std::span<partial::insured_t> const& insured,
	    std::function<void(std::string const&)> const& on_error);

	using search_term = std::variant<unsigned, std::string>;

	inline static std::vector<unsigned> search_insured_from_term(
	    search_term const& term,
	    std::span<partial::insured_t> const& insured,
	    std::function<void(std::string const&)> const& on_error) {
		if (std::holds_alternative<unsigned>(term)) {
			return search_insured_from_position(std::get<unsigned>(term),
			                                    insured, on_error);
		}

		return search_insured_from_keyword(std::get<std::string>(term), insured,
		                                   on_error);
	}
}  // namespace quick_dra
