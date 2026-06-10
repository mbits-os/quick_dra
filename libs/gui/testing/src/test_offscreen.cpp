// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "test_offscreen.hpp"
#include <filesystem>
#include <format>
#include <quick_dra/base/str.hpp>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using namespace std::literals;

std::string calc_offscreen() {
	auto path = std::filesystem::path{__FILE__}.parent_path().parent_path() / "offscreen.json"sv;
#ifdef WIN32
	path = std::filesystem::weakly_canonical(path);
	auto const root = path.root_name().native();
	auto const& root_relative = path.native().substr(root.size());
	path = root_relative;
#endif
	return std::format("offscreen:configfile={}", quick_dra::as_sv(path.generic_u8string()));
}

PlatformArgsStorage::PlatformArgsStorage(int& argc, char**& argv) {
	args.reserve(static_cast<size_t>(argc) + 1 + 2);
	args.push_back(argv[0]);
	args.push_back(platform.data());
	args.push_back(offscreen.data());

	for (auto arg : std::span<char*>{argv + 1, static_cast<size_t>(argc)}) {
		args.push_back(arg);
	}
	argc = static_cast<int>(args.size() - 1);
	argv = args.data();
}
