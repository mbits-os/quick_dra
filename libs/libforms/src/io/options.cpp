// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/std.h>
#include <array>
#include <map>
#include <optional>
#include <quick_dra/base/chrono.hpp>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/io/http.hpp>
#include <quick_dra/io/options.hpp>
#include <quick_dra/io/tax_config.hpp>
#include <quick_dra/version.hpp>
#include <string>
#include <utility>

namespace quick_dra {
	std::string set_filename(unsigned report_index, year_month const& date) {
		return fmt::format("quick-dra_{}{:02}-{:02}.xml", static_cast<int>(date.year()),
		                   static_cast<unsigned>(date.month()), report_index);
	}

	std::optional<config> parse_config(verbose level,
	                                   year_month const& date,
	                                   std::filesystem::path const& path,
	                                   std::optional<std::filesystem::path> const& tax_config_path,
	                                   github_config download) {
		auto result = config::parse_yaml(path);
		if (!result) return result;

		auto tax_cfg = load_tax_config(level, tax_config_path, download);
		if (!tax_cfg) {
			result.reset();
			return result;
		}

		auto& params = result->params;

		lookup_parameters(params, *tax_cfg, result->accident_insurance, date);

		tax_cfg->debug_print(level);
		result->debug_print(level);

		if (level >= verbose::names_and_summary) {
			bool everyone_has_salary = true;
			for (auto const& insured : result->insured) {
				if (!insured.lookup(date).salary) {
					everyone_has_salary = false;
					break;
				}
			}  // GCOV_EXCL_LINE[WIN32]
			if (!everyone_has_salary) {
				fmt::print("--   minimal pay for month reported: {:.02f} zł\n", result->params.minimal_pay);
			}
		}

		return result;
	}
}  // namespace quick_dra
