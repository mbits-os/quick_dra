// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <chrono>
#include <map>
#include <string>
#include <utility>

namespace quick_dra {
	using namespace std::chrono;
	using namespace std::literals;
	static constexpr auto null_month = 0y / 1;
	year_month_day get_today();
	std::string fmt_date(year_month const& ym);
	std::string fmt_date_slash(year_month const& ym);

	template <typename T>
	inline std::pair<year_month, T> find_in_timeline(year_month const& key, std::map<year_month, T> const& mapping) {
		std::pair<year_month, T> result = {null_month, T{}};

		for (auto const& [date, value] : mapping) {
			if (key < date) continue;
			if (result.first > date) continue;
			result = {date, value};
		}

		return result;
	}  // GCOV_EXCL_LINE[GCC]

	template <typename T>
	inline std::tuple<bool, year_month, T> find_in_timeline_opt(year_month const& key,
	                                                            std::map<year_month, T> const& mapping) {
		std::tuple<bool, year_month, T> result = {false, null_month, T{}};

		for (auto const& [date, value] : mapping) {
			if (key < date) continue;
			if (std::get<year_month>(result) > date) continue;
			result = {true, date, value};
		}

		return result;
	}  // GCOV_EXCL_LINE[GCC]

	inline year_month month_today() {
		auto const today = get_today();
		return today.year() / today.month();
	}

	template <typename T>
	inline year_month last_date_or_today(year_month const& key, std::map<year_month, T> const& mapping) {
		if (key != null_month) {
			return key;
		}
		if (mapping.empty()) {
			return month_today();
		}
		return mapping.rbegin()->first;
	}  // GCOV_EXCL_LINE[GCC]

	template <typename T>
	inline std::pair<year_month, T*> find_or_add_to_timeline(year_month const& key, std::map<year_month, T>& mapping) {
		auto const month = last_date_or_today(key, mapping);
		return {month, &mapping[month]};
	}  // GCOV_EXCL_LINE[GCC]
}  // namespace quick_dra
