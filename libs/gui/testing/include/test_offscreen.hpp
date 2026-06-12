// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QTest>
#include <string>
#include <vector>

using namespace std::literals;

std::string calc_offscreen();

struct PlatformArgsStorage {
	PlatformArgsStorage(int& argc, char**& argv);

private:
	std::string platform = "-platform"s;
	std::string offscreen = calc_offscreen();
	std::vector<char*> args{};
};

void loadFonts();

#define OFFSCREEN_PLATFORM_SETUP PlatformArgsStorage offscreenPlatform{argc, argv};

#define QTEST_OFFSCREEN_MAIN(TEST_CLASS) \
	QTEST_MAIN_WRAPPER(TEST_CLASS, OFFSCREEN_PLATFORM_SETUP QTEST_MAIN_SETUP() loadFonts();)
