// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/std.h>
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <optional>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/base/str.hpp>
#include <quick_dra/gui/dirfs.hpp>
#include <source_location>
#include <span>
#include <variant>

#undef stdout
#undef stderr

namespace quick_dra::gui::testing {
	class dirfs : public ::testing::Test {
	public:
		static void SetUpTestSuite() {
			auto const root_dir = platform::exec_dir() / "fs_root"sv / "web"sv;
			std::filesystem::create_directories(root_dir / "subdir"sv);

			{
				std::ofstream f{root_dir / "index.html"};
				f << "ROOT INDEX";
			}

			{
				std::ofstream f{root_dir / "subdir"sv / "index.html"};
				f << "SUBDIR INDEX";
			}

			{
				std::ofstream f{root_dir / ".."sv / "index.html"};
				f << "OUT OF BOUNDS";
			}

			directory_filesystem::set_global(root_dir);
		}
	};

	TEST_F(dirfs, respond) {
		auto const& dirfs = directory_filesystem::get_global();
		std::vector<char> stg{};
		ASSERT_TRUE(dirfs.respond("index.html", stg));
		ASSERT_TRUE(dirfs.respond("/index.html", stg));
		ASSERT_TRUE(dirfs.respond("/subdir", stg));
		ASSERT_TRUE(dirfs.respond("/subdir/", stg));
		ASSERT_TRUE(dirfs.respond("/subdir/../index.html", stg));

		ASSERT_EQ(dirfs.respond("index.html", stg)->contents, "ROOT INDEX"sv);
		ASSERT_EQ(dirfs.respond("/index.html", stg)->contents, "ROOT INDEX"sv);
		ASSERT_EQ(dirfs.respond("/subdir", stg)->redirect.value_or(""), "/subdir/"sv);
		ASSERT_EQ(dirfs.respond("/subdir/", stg)->contents, "SUBDIR INDEX"sv);
		ASSERT_EQ(dirfs.respond("/subdir/../index.html", stg)->contents, "ROOT INDEX"sv);

		ASSERT_FALSE(dirfs.respond("../index.html", stg));
		ASSERT_FALSE(dirfs.respond("/subdir/../../index.html", stg));
	}
}  // namespace quick_dra::gui::testing