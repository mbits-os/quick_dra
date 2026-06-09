// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QTest>
#include <format>
#include <quick_dra/base/str.hpp>
#include <span>
#include <vector>
#include "../../../test_offscreen.hpp"
#include "GuiTest.hpp"

using namespace std::literals;

struct PlatformArgsStorage {
	PlatformArgsStorage(int& argc, char**& argv) {
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

private:
	std::string platform = "-platform"s;
	std::string offscreen = calc_offscreen();
	std::vector<char*> args{};
};

int main(int argc, char* argv[]) {
	PlatformArgsStorage offscreenPlatform{argc, argv};
	TESTLIB_SELFCOVERAGE_START("GuiTest");
	QTest::Internal::callInitMain<GuiTest>();
	QTEST_MAIN_SETUP();
	QTEST_SET_MAIN_SOURCE_PATH
	GuiTest tc{};
	return QTest::qExec(&tc, argc, argv);
}
