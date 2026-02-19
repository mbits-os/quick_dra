// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <fmt/std.h>
#include <gtest/gtest.h>
#include <curl/mock.hpp>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/io/options.hpp>
#include <span>
#include <sstream>

#define GITHUB_MAIN_REF \
	"https://raw.githubusercontent.com/mbits-os/quick_dra/refs/heads/main"

namespace quick_dra::testing {
	using std::literals::operator""s;
	using std::literals::operator""sv;
	using std::literals::operator""y;

	class ImplNoTaxConfig : public CURL {
		CURLcode perform() override {
			set_response(404, ""sv, {{"content-type"sv, "text/plain"sv}});
			return CURLE_OK;
		}
	};

	TEST(config, no_tax_config) {
		auto const here = platform::exec_dir();
		// reverse of build/<config>/bin/tests
		auto const root =
		    here.parent_path().parent_path().parent_path().parent_path();
		auto const config_path =
		    root / "libs"sv / "cli"sv / "tests"sv / "data"sv /
		    ".quick_dra.AB4123456_50671500000.quarter.yaml"sv;
		auto const custom_tax_config = here / "nonexisting-file"sv;

		set_curl_factory<ImplNoTaxConfig>();

		::testing::internal::CaptureStdout();
		auto const cfg = parse_config(verbose::parameters, 2026y / 1,
		                              config_path, custom_tax_config);
		auto const log = ::testing::internal::GetCapturedStdout();
		auto const expected_log = fmt::format(
		    "Quick-DRA: error: cannot find {}\n"
		    "-- nothing downloaded (status: 404, {})\n"
		    "Quick-DRA: error: cannot find {}\n"sv,
		    custom_tax_config, GITHUB_MAIN_REF "/data/config/tax_config.yaml"sv,
		    platform::config_data_dir() / "tax_config.yaml"sv);
		ASSERT_FALSE(cfg);
		ASSERT_EQ(log, expected_log);
	}
}  // namespace quick_dra::testing
