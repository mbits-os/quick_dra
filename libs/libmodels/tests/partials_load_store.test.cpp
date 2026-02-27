// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <fmt/std.h>
#include <gtest/gtest.h>
#include <array>
#include <fstream>
#include <map>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/models/project_reader.hpp>
#include <ranges>
#include <span>
#include <utility>
#include <variant>
#include <vector>
#include <yaml/parser.hpp>

namespace quick_dra::testing {
	auto const& project_root() {
		// reverse of build/<config>/bin/tests
		static auto root = platform::exec_dir()
		                       .parent_path()
		                       .parent_path()
		                       .parent_path()
		                       .parent_path();
		return root;
	}
	auto const& test_data_dir() {
		static auto dir =
		    project_root() / "libs"sv / "cli"sv / "tests"sv / "data"sv;
		return dir;
	}

	TEST(partial, file_not_found) {
		auto const filename = test_data_dir() / "no-such"sv;
		::testing::internal::CaptureStderr();
		partial::config::load_partial(filename, true);
		auto err = ::testing::internal::GetCapturedStderr();

		EXPECT_EQ(err,
		          fmt::format("Quick-DRA: file {} will be created as needed.\n",
		                      filename));

		::testing::internal::CaptureStderr();
		partial::config::load_partial(filename, false);
		err = ::testing::internal::GetCapturedStderr();

		EXPECT_EQ(err, ""sv);
	}

	std::filesystem::path unique_name(std::string_view name) {
		auto const basename = std::filesystem::temp_directory_path() / name;
		auto filename = basename;
		auto counter = 0u;

		while (exists(filename)) {
			++counter;
			filename = fmt::format("{}-{}", basename, counter);
		}

		return filename;
	}

	std::filesystem::path unique(std::string_view name,
	                             std::string_view contents = {}) {
		auto filename = unique_name(name);

		std::ofstream out{filename, std::ios::out | std::ios::binary};
		out.write(contents.data(),
		          static_cast<std::streamsize>(contents.size()));

		return filename;
	}

	TEST(partial, file_not_readable) {
		using namespace std::filesystem;

#ifdef _WIN32
		auto const filename = unique_name("write_only_file"sv);
		create_directories(filename);
#else
		auto const filename = unique("write_only_file"sv);
		permissions(filename,
		            perms::owner_read | perms::group_read | perms::others_read,
		            perm_options::remove);
#endif
		EXPECT_DEATH(partial::config::load_partial(filename),
		             "Quick-DRA: error: could not read .*write_only_file.*\n");
		remove(filename);
	}

	TEST(partial, errors_encountered) {
		auto const filename = unique("version-is-john"sv, "wersja: John\n"sv);
		EXPECT_DEATH(partial::config::load_partial(filename),
		             "Quick-DRA: error: .*version-is-john.* needs to be "
		             "updated before continuing\n");
		remove(filename);
	}

	TEST(partial, fully_read) {
		auto const filename =
		    test_data_dir() / ".quick_dra.AB4123456_50671500000.quarter.yaml"sv;
		::testing::internal::CaptureStderr();
		partial::config::load_partial(filename);
		auto err = ::testing::internal::GetCapturedStderr();

		EXPECT_EQ(err, ""sv);
	}

	TEST(partial, store) {
		auto const filename = unique("partial-config-store"sv);
		auto cfg = partial::config::load_partial(
		    test_data_dir() /
		    ".quick_dra.AB4123456_50671500000.quarter.yaml"sv);
		EXPECT_TRUE(cfg.store(filename));
		std::filesystem::remove(filename);
	}

	TEST(partial, store_error) {
		auto const dirname = test_data_dir();
		auto cfg = partial::config::load_partial(
		    test_data_dir() /
		    ".quick_dra.AB4123456_50671500000.quarter.yaml"sv);
		EXPECT_FALSE(cfg.store(dirname));
	}
}  // namespace quick_dra::testing
