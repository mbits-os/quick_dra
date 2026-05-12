// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <array>
#include <quick_dra/base/chrono.hpp>
#include <string>
#include <string_view>

using namespace std::literals;

namespace quick_dra {
	namespace {
		static constexpr auto months = std::array{
		    "January"sv, "February"sv, "March"sv,     "April"sv,   "May"sv,      "June"sv,
		    "July"sv,    "August"sv,   "September"sv, "October"sv, "November"sv, "December"sv,
		};
	}

	year_month_day get_today() {
		auto const now = system_clock::now();
		auto const local = floor<days>(zoned_time{current_zone(), now}.get_local_time());
		return year_month_day{local};
	}

	std::string fmt_date(year_month const& ym) {
		if (ym == null_month) {
			return "\"default\" month"s;
		}
		if (ym.month().ok() && ym.year() > 0y) {
			auto const year = static_cast<int>(ym.year());
			auto const month = static_cast<unsigned>(ym.month());
			return fmt::format("{} {}", months[month - 1], year);
		}
		return fmt_date_slash(ym);
	}

	std::string fmt_date_slash(year_month const& ym) {
		auto const year = static_cast<int>(ym.year());
		auto const month = static_cast<unsigned>(ym.month());
		return fmt::format("{:04}/{:02}", year, month);
	}
}  // namespace quick_dra
