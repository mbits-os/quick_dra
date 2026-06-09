// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <filesystem>
#include <format>
#include <quick_dra/base/str.hpp>

inline std::string calc_offscreen() {
	using std::literals::operator""sv;

	auto path = std::filesystem::path{__FILE__}.parent_path() / "test-offscreen.json"sv;
#ifdef WIN32
	path = std::filesystem::weakly_canonical(path);
	auto const root = path.root_name().native();
	auto const& root_relative = path.native().substr(root.size());
	path = root_relative;
#endif
	return std::format("offscreen:configfile={}", quick_dra::as_sv(path.generic_u8string()));
}
