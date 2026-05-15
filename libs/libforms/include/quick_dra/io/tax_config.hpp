// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <filesystem>
#include <optional>
#include <quick_dra/base/chrono.hpp>
#include <quick_dra/base/verbose.hpp>
#include <quick_dra/models/types.hpp>

namespace quick_dra {
	std::optional<tax_config> load_tax_config(verbose level,
	                                          std::optional<std::filesystem::path> const& tax_config_path);
	void lookup_parameters(tax_parameters& out,
	                       tax_config const& config,
	                       std::optional<std::map<std::chrono::year_month, percent>> const& accident_insurance_override,
	                       year_month const& key);
}  // namespace quick_dra
