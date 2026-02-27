// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "ctre_parsing.hpp"
#include <chrono>
#include <concepts>
#include <ctre.hpp>
#include <optional>
#include <quick_dra/base/types.hpp>
#include <string>
#include <string_view>

namespace quick_dra::builtin::xml {
	std::optional<std::chrono::year_month_day> parse_date(
	    std::optional<std::string> const& arg) {
		if (!arg) {
			return std::nullopt;
		}
		auto const m = ctre::match<
		    "^"
		    "([12][0-9]{3})-"
		    "([1-9]|0[0-9]|1[012])-"
		    "([1-9]|[012][0-9]|3[01])"
		    "$">(*arg);
		if (auto const& [whole, year_str, month_str, day_str] = m; whole) {
			int year{};
			int month{};
			int day{};
			from_chars(year_str, year);
			from_chars(month_str, month);
			from_chars(day_str, day);

			auto const result = std::chrono::year{year} / month / day;
			if (!result.ok()) {
				return std::nullopt;
			}

			return result;
		}
		return std::nullopt;
	}
}  // namespace quick_dra::builtin::xml
