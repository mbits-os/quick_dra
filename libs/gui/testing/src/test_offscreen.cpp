// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "test_offscreen.hpp"
#include <QApplication>
#include <QFontDatabase>
#include <filesystem>
#include <format>
#include <print>
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

void loadFonts() {
	auto path = std::filesystem::path{__FILE__}.parent_path().parent_path() / "res"sv / "fonts"sv;
	for (auto const& entry : std::filesystem::recursive_directory_iterator{path}) {
		if (entry.is_directory()) continue;
		auto const ext = quick_dra::as_str(entry.path().extension().generic_u8string());
		if (ext != ".ttf"sv && ext != ".otf"sv) continue;
		QFontDatabase::addApplicationFont(QString::fromUtf8(quick_dra::as_sv(entry.path().generic_u8string())));
	}

	QFont testFont{"DejaVu Sans"};
	testFont.setPointSizeF(9);
	testFont.setHintingPreference(QFont::PreferFullHinting);  // Prevents anti-aliasing bugs on some platforms
	qApp->setFont(testFont);
}
