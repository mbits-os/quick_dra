// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <chrono>
#include <optional>
#include <string>

namespace quick_dra::builtin::xml {
	std::optional<std::chrono::year_month_day> parse_date(
	    std::optional<std::string> const& arg);
}  // namespace quick_dra::builtin::xml
