// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <quick_dra/base/chrono.hpp>

namespace quick_dra {
	year_month_day get_today() {
		auto const now = system_clock::now();
		auto const local = floor<days>(zoned_time{current_zone(), now}.get_local_time());
		return year_month_day{local};
	}
}  // namespace quick_dra
