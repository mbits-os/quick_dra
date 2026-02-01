// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <filesystem>
#include <quick_dra/base/dir_names.hpp>
#include <string_view>

using namespace std::literals;

namespace quick_dra::platform {
	std::filesystem::path _exec_path();

	inline std::filesystem::path const& exec_path() {
		static auto const path = _exec_path();
		return path;
	}

	inline std::filesystem::path const& exec_dir() {
		static auto const path = exec_path().parent_path();
		return path;
	}

	inline std::filesystem::path const& config_data_dir() {
		static auto const path =
		    exec_path().parent_path().parent_path() / dir_names::config;
		return path;
	}

	std::filesystem::path const& home_path();

	inline std::filesystem::path get_config_path(
	    std::optional<std::string> const& override) {
		if (override) return *override;

		return home_path() / ".quick_dra.yaml"sv;
	}

}  // namespace quick_dra::platform
