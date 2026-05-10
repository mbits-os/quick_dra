// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <array>
#include <optional>
#include <quick_dra/base/types.hpp>
#include <string>
#include <string_view>

namespace quick_dra::gui {
	std::string_view document_kind(char kind);
	std::string ratio_from(unsigned num, unsigned den);
	std::string info_span(std::string_view label, std::string_view value);

	std::string document_info(char kind, std::optional<std::string> const& document);
	std::string document_info(std::optional<std::string> const& kind, std::optional<std::string> const& document);
	std::string insurance_title_info(std::optional<insurance_title> const& title);
	std::string salary_info(year_month const& month,
	                        std::optional<ratio> const& scale,
	                        std::optional<currency> const& salary);
	std::string name_from(std::optional<std::string> const& first_name,
	                      std::optional<std::string> const& last_name,
	                      bool markdown);

	inline std::string second_line(std::convertible_to<std::string_view> auto... items) {
		const auto input = std::array{std::string_view{items}...};
		size_t pos = input.size() + 1;
		size_t count = 0;
		size_t length = 0;
		size_t index = 0;
		for (auto const& item : input) {
			if (item.empty()) {
				++index;
				continue;
			}
			if (pos > input.size()) {
				pos = index;
			}
			++count;
			++index;
			length += item.length();
		}

		if (!count) {
			return {};
		}
		if (count == 1) {
			return as_str(input[pos]);
		}

		static constexpr auto sep = ", "sv;
		std::string result{};
		result.reserve(length + sep.length() * (count - 1));
		for (auto const& item : input) {
			if (item.empty()) continue;
			if (!result.empty()) {
				result.append(sep);
			}
			result.append(item);
		}
		return result;
	}
}  // namespace quick_dra::gui
