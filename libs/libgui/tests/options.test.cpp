// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/std.h>
#include <gtest/gtest.h>
#include <quick_dra/gui/options/options.hpp>
#include <span>

namespace quick_dra::testing {
	struct testcase {
		std::string_view args{};
		std::string_view output{};

		friend std::ostream& operator<<(std::ostream& out, testcase const& tc) {
			return out << join(split_sv(tc.args, '\0'_sep), ' '_sep);
		}
	};

	struct arglist {
		std::vector<std::string> stg{};
		std::vector<char*> argv{};

		arglist(std::span<std::string_view const> const& items) {
			stg.reserve(items.size() + 1);
			stg.push_back("qdra-qui"s);
			std::transform(items.begin(), items.end(), std::back_inserter(stg), conv<std::string, std::string_view>);

			argv.reserve(stg.size() + 1);
			std::transform(stg.begin(), stg.end(), std::back_inserter(argv), [](auto& arg) { return arg.data(); });
			argv.push_back(nullptr);
		}

		args::args_view as_args() { return args::from_main({static_cast<unsigned>(stg.size()), argv.data()}); };
		static arglist from(testcase const& params) {
			if (params.args.empty()) return {{}};
			auto const args = split_sv(params.args, '\0'_sep);
			// for (auto const& arg : args) {
			// 	fmt::print("[{}]\n", arg);
			// }
			return {args};
		}
	};

	class opts : public ::testing::TestWithParam<testcase> {};

	void print_debug(debug_options const& debug) {
		for (auto const& [key, value] : debug.options) {
			fmt::print("--debug:{} = {}\n", key, value);
		}

		for (auto const& [key, value] : debug.flags) {
			fmt::print("--debug:{} = {}\n", key, value);
		}
	}

	TEST_P(opts, parse) {
		auto const enabled = get_debug_suite().enabled;

		auto const& params = GetParam();

		if (enabled || params.args.empty()) {
			auto const opts = options::parse(arglist::from(params).as_args());
			::testing::internal::CaptureStdout();
			print_debug(opts.debug);
			auto const output = ::testing::internal::GetCapturedStdout();

			ASSERT_EQ(params.output, output);
		} else {
			ASSERT_DEATH(options::parse(arglist::from(params).as_args()),
			             "qdra-qui: error: unrecognized argument: --debug:");
		}
	}

	TEST_F(opts, option_implies) {
		static constexpr debug_option test_options[] = {
		    {
		        .name = "option"sv,
		        .meta = "<val>"sv,
		        .description = "anything"sv,
		        .implies = "flag-a,no-flag-b"sv,
		    },
		};

		static constexpr debug_flag test_flags[] = {
		    {
		        .name = "flag-a"sv,
		        .description = "anything"sv,
		        .opposite = "anything"sv,
		    },
		    {
		        .name = "flag-b"sv,
		        .description = "anything"sv,
		        .opposite = "anything"sv,
		    },
		};

		static constexpr debug_suite suite{
		    .known_options = test_options,
		    .known_flags = test_flags,
		};

		auto const params = testcase{
		    "--debug:option\0value"sv,
		    "--debug:option = value\n"
		    "--debug:flag-a = true\n"
		    "--debug:flag-b = false\n"sv,
		};
		auto const opts = options::parse(arglist::from(params).as_args(), suite);
		::testing::internal::CaptureStdout();
		print_debug(opts.debug);
		auto const output = ::testing::internal::GetCapturedStdout();

		ASSERT_EQ(params.output, output);
	}

	TEST_F(opts, flag_implies) {
		static constexpr debug_flag test_flags[] = {
		    {
		        .name = "flag-a"sv,
		        .description = "anything"sv,
		        .opposite = "anything"sv,
		    },
		    {
		        .name = "flag-b"sv,
		        .description = "anything"sv,
		        .opposite = "anything"sv,
		        .implies = "flag-a"sv,
		    },
		};

		static constexpr debug_suite suite{
		    .known_flags = test_flags,
		};

		auto const params = testcase{
		    "--debug:flag-b"sv,
		    "--debug:flag-a = true\n"
		    "--debug:flag-b = true\n"sv,
		};
		auto const opts = options::parse(arglist::from(params).as_args(), suite);
		::testing::internal::CaptureStdout();
		print_debug(opts.debug);
		auto const output = ::testing::internal::GetCapturedStdout();

		ASSERT_EQ(params.output, output);
	}

	static constexpr testcase tests[] = {
	    {},
	    {"--debug:app-directory\0."sv, "--debug:app-directory = .\n"sv},
	    {"--debug:browser"sv,
	     "--debug:browser = true\n"
#ifdef GUI_DEBUG_FRAMELESS
	     "--debug:frameless = false\n"
#endif
	     ""sv},
	};

	INSTANTIATE_TEST_SUITE_P(test, opts, ::testing::ValuesIn(tests));

}  // namespace quick_dra::testing
