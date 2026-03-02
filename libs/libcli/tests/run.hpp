// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/std.h>
#include <gtest/gtest.h>
#include <filesystem>
#include <optional>
#include <quick_dra/cli/builtins.hpp>
#include <span>
#include <variant>

#undef stdout
#undef stderr

namespace quick_dra::builtin::testing {
	struct new_file {
		std::string_view name;
		std::string_view cmp;
	};

	enum class compare { begin, end, all };
	using std::filesystem::perms;
	static constexpr auto readonly_perms =
	    perms::owner_read | perms::group_read | perms::others_read;

	struct runnable_testcase {
		std::string_view name{};
		std::string_view args{};
		std::span<std::string_view const> post{};

		std::string_view config_name{};
		std::string_view config{};

		std::variant<std::string_view, std::string (*)()> stdout{};
		std::variant<std::string_view, std::string (*)()> stderr{};

		std::optional<new_file> writes{};

		int returncode{0};
		compare check_stdout{compare::all};
		compare check_stderr{compare::all};
		perms mode{perms::none};

		bool run_test() const;

		friend std::ostream& operator<<(std::ostream& out,
		                                runnable_testcase const& test) {
			if (!test.name.empty()) return out << test.name;
			return out << test.args;
		}
	};

	class cli_test : public ::testing::TestWithParam<runnable_testcase> {};

}  // namespace quick_dra::builtin::testing
