// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <quick_dra/base/chrono.hpp>
#include <quick_dra/base/verbose.hpp>
#include <quick_dra/models/types.hpp>

namespace quick_dra {
	enum class github_config : bool { skip = false, download = true };
	std::optional<tax_config> load_tax_config(verbose level,
	                                          std::optional<std::filesystem::path> const& tax_config_path,
	                                          github_config download = github_config::download);
	void lookup_parameters(tax_parameters& out,
	                       tax_config const& config,
	                       std::optional<std::map<std::chrono::year_month, percent>> const& accident_insurance_override,
	                       year_month const& key);
}  // namespace quick_dra
