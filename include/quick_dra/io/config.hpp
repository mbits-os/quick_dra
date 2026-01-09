// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <chrono>
#include <filesystem>
#include <optional>
#include <quick_dra/base/verbose.hpp>
#include <quick_dra/model/types.hpp>
#include <string>

namespace quick_dra {
	using namespace std::chrono;

	struct options {
		std::filesystem::path config_path{};
		verbose verbose_level{};
		year_month_day today{};
		unsigned report_index{};
		year_month date{};
		bool indent_xml{};

		static options from_cli(int argc, char* argv[]);
	};

	std::string set_filename(unsigned report_index, year_month const& date);
	std::optional<config> parse_config(verbose level,
	                                   year_month const& date,
	                                   std::filesystem::path const& path);
}  // namespace quick_dra
