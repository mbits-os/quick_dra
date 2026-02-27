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

	class ImplSimpleTaxConfig : public CURL {
		CURLcode perform() override {
			set_response(200, R"(version: 1

scale:
  2022/1:
    30000 zł: 17%
    120000 zł: 32%
  2022/7:
    30000 zł: 12%
    120000 zł: 32%

minimal-pay:
  2022/01: 3010 zł
  2023/01: 3490 zł
  2023/07: 3600 zł
  2024/01: 4242 zł
  2024/07: 4300 zł
  2025/01: 4666 zł
  2026/01: 4806 zł

costs-of-obtaining:
  2022/01: { local: 250 zł, remote: 300 zł }

contributions:
  2022/01:
    emerytalne: { płatnik: 9.76%, ubezpieczony: 9.76% }
    rentowe: { płatnik: 6.5%, ubezpieczony: 1.5% }
    chorobowe: { ubezpieczony: 2.45% }
    wypadkowe: { płatnik: 1.67% }
    zdrowotne: { ubezpieczony: 9% }
)"sv,
			             {{"content-type"sv, "application/yaml"sv}});
			return CURLE_OK;
		}
	};

	TEST(config, filename) {
		EXPECT_EQ(set_filename(1, 2016y / 1), "quick-dra_201601-01.xml"sv);
		EXPECT_EQ(set_filename(99, 1999y / 12), "quick-dra_199912-99.xml"sv);
	}

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

	TEST(config, custom_tax_config) {
		auto const here = platform::exec_dir();
		// reverse of build/<config>/bin/tests
		auto const root =
		    here.parent_path().parent_path().parent_path().parent_path();
		auto const config_path =
		    root / "libs"sv / "cli"sv / "tests"sv / "data"sv /
		    ".quick_dra.AB4123456_50671500000.quarter.yaml"sv;
		auto const custom_tax_config =
		    root / "data"sv / "config"sv / "tax_config.yaml";

		set_curl_factory<ImplNoTaxConfig>();

		::testing::internal::CaptureStdout();
		auto const cfg = parse_config(verbose::parameters, 2026y / 1,
		                              config_path, custom_tax_config);
		auto const log = ::testing::internal::GetCapturedStdout();
		auto const expected_log = fmt::format(
		    "-- nothing downloaded (status: 404, {})\n"
		    "Quick-DRA: error: cannot find {}\n"
		    "{}"sv,
		    GITHUB_MAIN_REF "/data/config/tax_config.yaml"sv,
		    platform::config_data_dir() / "tax_config.yaml"sv,
		    R"(-- costs of obtaining per month:
--   2022-01: 250 zł / 300 zł
-- minimal pay per month:
--   2022-01: 3010 zł
--   2023-01: 3490 zł
--   2023-07: 3600 zł
--   2024-01: 4242 zł
--   2024-07: 4300 zł
--   2025-01: 4666 zł
--   2026-01: 4806 zł
-- tax scale per month:
--   2022-01:
--     over 30000 zł at 17%
--     over 120000 zł at 32%
--   2022-07:
--     over 30000 zł at 12%
--     over 120000 zł at 32%
-- insurance rates per month:
--   2022-01:
--     health: insured 9%
--     pension insurance: payer 9.76%, insured 9.76%
--     disability insurance: payer 6.5%, insured 1.5%
--     health insurance: insured 2.45%
--     accident insurance: payer 1.67%
-- payer:
--   name: Jan Nowak
--   social id: 26211012346
--   tax id: 7680002466
--   ident: 2 AB4123456
-- insured:
--   - name: Piotr Iksiński
--     insurance title: 0110 0 0
--     ident: P 50671500000
--     salary: 1/4 of <minimal pay>
-- parameters
--   cost of obtaining: 250 zł / 300 zł
--   health: insured 9%
--   pension insurance: payer 9.76%, insured 9.76%
--   disability insurance: payer 6.5%, insured 1.5%
--   health insurance: insured 2.45%
--   accident insurance: payer 1.67%
--   tax scale for month reported:
--     over 30000 zł at 12%
--     over 120000 zł at 32%
--   minimal pay for month reported: 4806.00 zł
)"sv);
		ASSERT_TRUE(cfg);
		ASSERT_EQ(log, expected_log);
	}

	TEST(config, network_tax_config) {
		auto const here = platform::exec_dir();
		// reverse of build/<config>/bin/tests
		auto const root =
		    here.parent_path().parent_path().parent_path().parent_path();
		auto const config_path =
		    root / "libs"sv / "cli"sv / "tests"sv / "data"sv /
		    ".quick_dra.AB4123456_50671500000.quarter.yaml"sv;

		set_curl_factory<ImplSimpleTaxConfig>();

		::testing::internal::CaptureStdout();
		auto const cfg = parse_config(verbose::parameters, 2026y / 1,
		                              config_path, std::nullopt);
		auto const log = ::testing::internal::GetCapturedStdout();
		auto const expected_log = fmt::format(
		    "-- downloaded {}\n"
		    "Quick-DRA: error: cannot find {}\n"
		    "{}"sv,
		    GITHUB_MAIN_REF "/data/config/tax_config.yaml"sv,
		    platform::config_data_dir() / "tax_config.yaml"sv,
		    R"(-- costs of obtaining per month:
--   2022-01: 250 zł / 300 zł
-- minimal pay per month:
--   2022-01: 3010 zł
--   2023-01: 3490 zł
--   2023-07: 3600 zł
--   2024-01: 4242 zł
--   2024-07: 4300 zł
--   2025-01: 4666 zł
--   2026-01: 4806 zł
-- tax scale per month:
--   2022-01:
--     over 30000 zł at 17%
--     over 120000 zł at 32%
--   2022-07:
--     over 30000 zł at 12%
--     over 120000 zł at 32%
-- insurance rates per month:
--   2022-01:
--     health: insured 9%
--     pension insurance: payer 9.76%, insured 9.76%
--     disability insurance: payer 6.5%, insured 1.5%
--     health insurance: insured 2.45%
--     accident insurance: payer 1.67%
-- payer:
--   name: Jan Nowak
--   social id: 26211012346
--   tax id: 7680002466
--   ident: 2 AB4123456
-- insured:
--   - name: Piotr Iksiński
--     insurance title: 0110 0 0
--     ident: P 50671500000
--     salary: 1/4 of <minimal pay>
-- parameters
--   cost of obtaining: 250 zł / 300 zł
--   health: insured 9%
--   pension insurance: payer 9.76%, insured 9.76%
--   disability insurance: payer 6.5%, insured 1.5%
--   health insurance: insured 2.45%
--   accident insurance: payer 1.67%
--   tax scale for month reported:
--     over 30000 zł at 12%
--     over 120000 zł at 32%
--   minimal pay for month reported: 4806.00 zł
)"sv);
		ASSERT_TRUE(cfg);
		ASSERT_EQ(log, expected_log);
	}
}  // namespace quick_dra::testing
