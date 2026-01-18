// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <chrono>
#include <quick_dra/base/verbose.hpp>
#include <quick_dra/models/types.hpp>
#include <string>
#include <vector>

namespace quick_dra {
	struct form {
		std::string key{};
		global_object state{};
		std::vector<calculated_section> fill(
		    verbose level,
		    std::vector<compiled_section> const& tmplt) const;
	};

	std::vector<form> prepare_form_set(verbose level,
	                                   unsigned report_index,
	                                   std::chrono::year_month const& date,
	                                   std::chrono::year_month_day const& today,
	                                   config const& cfg);
}  // namespace quick_dra
