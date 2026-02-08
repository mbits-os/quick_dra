// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <args_parser.hpp>
#include <filesystem>
#include <quick_dra/models/types.hpp>
#include <vector>

namespace quick_dra::builtin::insured::remove {
	struct options {
		std::filesystem::path path{};
		std::vector<unsigned> found{};
		bool ask_questions{true};
		partial::config cfg{};
	};

	int get_options(args::parser& parser, options& out);
}  // namespace quick_dra::builtin::insured::remove
