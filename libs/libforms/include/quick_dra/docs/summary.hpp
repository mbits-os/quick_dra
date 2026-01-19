// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <optional>
#include <quick_dra/models/base_types.hpp>
#include <string>
#include <utility>
#include <vector>

namespace quick_dra {
	struct form;

	struct summary_line {
		std::string label;
		std::optional<currency> value;
	};

	std::vector<summary_line> gather_summary_data(
	    std::vector<quick_dra::form> const& forms);

	void print_summary(std::vector<summary_line> const& rows);
}  // namespace quick_dra
