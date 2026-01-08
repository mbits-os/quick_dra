// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <filesystem>
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

	inline std::filesystem::path const& data_dir() {
		static auto const path = exec_path().parent_path() / "quick_dra_data"sv;
		return path;
	}

	std::filesystem::path const& home_path();
}  // namespace quick_dra::platform
