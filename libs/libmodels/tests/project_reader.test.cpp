// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <array>
#include <quick_dra/models/project_reader.hpp>
#include <span>
#include <utility>
#include <vector>

namespace quick_dra {
	void PrintTo(currency curr, std::ostream* os) {
		*os << fmt::format("{} zł", curr);
	}

	void PrintTo(percent per, std::ostream* os) {
		*os << fmt::format("{}%", per);
	}

	void PrintTo(ratio r, std::ostream* os) {
		*os << fmt::format("{}/{}", r.num, r.den);
	}
}  // namespace quick_dra

namespace quick_dra::testing {
	using std::literals::operator""y;

	class project_reader : public ::testing::Test {
	protected:
		std::string log{};
		std::string_view empty_log{};

		template <typename FileObj>
		std::optional<FileObj> read(std::string_view text) {
			::testing::internal::CaptureStderr();
			auto result = parser::parse_yaml_text<FileObj>(
			    {text.data(), text.size()}, "input"s);
			log = ::testing::internal::GetCapturedStderr();
			return result;
		}
	};

	TEST_F(project_reader, percent) {
		std::optional<percent> value{};

		// null
		value = read<percent>(""sv);
		ASSERT_FALSE(value);
		ASSERT_EQ(log, empty_log);

		// empty
		value = read<percent>("''"sv);
		ASSERT_FALSE(value);
		ASSERT_EQ(log, empty_log);

		// non-parsable
		value = read<percent>("word%"sv);
		ASSERT_FALSE(value);
		ASSERT_EQ(log,
		          "input:1:1: error: could not parse the percent value\n"sv);

		value = read<percent>("15,678$"sv);
		ASSERT_FALSE(value);
		ASSERT_EQ(log,
		          "input:1:1: error: could not parse the percent value\n"sv);

		// valid
		value = read<percent>("15,678%"sv);
		ASSERT_TRUE(value);
		ASSERT_EQ(*value, 15.68_per);
		ASSERT_EQ(log, empty_log);

		value = read<percent>("135.5 %"sv);
		ASSERT_TRUE(value);
		ASSERT_EQ(*value, 135.50_per);
		ASSERT_EQ(log, empty_log);
	}

	TEST_F(project_reader, currency) {
		std::optional<currency> value{};

		// null
		value = read<currency>(""sv);
		ASSERT_FALSE(value);
		ASSERT_EQ(log, empty_log);

		// empty
		value = read<currency>("''"sv);
		ASSERT_FALSE(value);
		ASSERT_EQ(log, empty_log);

		// non-parsable
		value = read<currency>("word zł"sv);
		ASSERT_FALSE(value);
		ASSERT_EQ(log,
		          "input:1:1: error: could not parse the currency value\n"sv);

		value = read<currency>("15,678$"sv);
		ASSERT_FALSE(value);
		ASSERT_EQ(log,
		          "input:1:1: error: could not parse the currency value\n"sv);

		// valid
		value = read<currency>("15,678 PLN"sv);
		ASSERT_TRUE(value);
		ASSERT_EQ(*value, 15.68_PLN);
		ASSERT_EQ(log, empty_log);

		value = read<currency>("135.5PLN"sv);
		ASSERT_TRUE(value);
		ASSERT_EQ(*value, 135.50_PLN);
		ASSERT_EQ(log, empty_log);

		auto const expected_map = std::map{
		    std::pair{50_PLN, "fifty"s},
		    std::pair{1000_PLN, "grand"s},
		};
		auto map = read<std::map<currency, std::string>>(
		    "50 zł: fifty\n"
		    "1000zł: grand\n"
		    ""sv);
		ASSERT_TRUE(map);
		ASSERT_EQ(*map, expected_map);
		ASSERT_EQ(log, empty_log);

		map = read<std::map<currency, std::string>>(
		    "50 zł: fifty\n"
		    "1000zł: grand\n"
		    "many zł: else"sv);
		ASSERT_FALSE(map);
		ASSERT_EQ(log,
		          "input:3:1: error: could not parse the currency value\n"sv);
	}

	TEST_F(project_reader, ratio) {
		std::optional<ratio> value{};

		// null
		value = read<ratio>(""sv);
		ASSERT_FALSE(value);
		ASSERT_EQ(log, empty_log);

		// empty
		value = read<ratio>("''"sv);
		ASSERT_FALSE(value);
		ASSERT_EQ(log, empty_log);

		// non-parsable
		value = read<ratio>("15"sv);
		ASSERT_FALSE(value);
		ASSERT_EQ(log, "input:1:1: error: expecting N/M, e.g. 1/1 or 4/5\n"sv);

		value = read<ratio>("alfa/beta"sv);
		ASSERT_FALSE(value);
		ASSERT_EQ(log,
		          "input:1:1: error: expecting a positive number\n"
		          "input:1:1: error: expecting N/M, e.g. 1/1 or 4/5\n"sv);

		value = read<ratio>("1/beta"sv);
		ASSERT_FALSE(value);
		ASSERT_EQ(log,
		          "input:1:1: error: expecting a positive number\n"
		          "input:1:1: error: expecting N/M, e.g. 1/1 or 4/5\n"sv);

		value = read<ratio>("alfa/1"sv);
		ASSERT_FALSE(value);
		ASSERT_EQ(log,
		          "input:1:1: error: expecting a positive number\n"
		          "input:1:1: error: expecting N/M, e.g. 1/1 or 4/5\n"sv);

		// valid
		value = read<ratio>("3/4"sv);
		ASSERT_TRUE(value);
		ASSERT_EQ(*value, ratio(3, 4));
		ASSERT_EQ(log, empty_log);

		value = read<ratio>("5/5"sv);
		ASSERT_TRUE(value);
		ASSERT_EQ(*value, ratio(8, 8));
		ASSERT_EQ(log, empty_log);
	}

	TEST_F(project_reader, insurance_title) {
		std::optional<insurance_title> value{};

		// null
		value = read<insurance_title>(""sv);
		ASSERT_FALSE(value);
		ASSERT_EQ(log, empty_log);

		// empty
		value = read<insurance_title>("''"sv);
		ASSERT_FALSE(value);
		ASSERT_EQ(log, empty_log);

		// non-parsable
		value = read<insurance_title>("words separated by spaces"sv);
		ASSERT_FALSE(value);
		ASSERT_EQ(
		    log,
		    "input:1:1: error: could not parse the insurance title value\n"sv);

		// valid
		insurance_title const expected = {
		    .title_code{"1234"s}, .pension_right{5}, .disability_level{6}};
		value = read<insurance_title>("1234 5 6"sv);
		ASSERT_TRUE(value);
		ASSERT_EQ(*value, expected);
		ASSERT_EQ(log, empty_log);
	}

	TEST_F(project_reader, year_month) {
		using ym_map = std::map<std::chrono::year_month, std::string>;
		std::optional<ym_map> value{};

		// non-parsable
		value = read<ym_map>("word: value"sv);
		ASSERT_FALSE(value);
		ASSERT_EQ(log, "input:1:1: error: expecting YYYY/MM or YYYY-MM\n"sv);

		value = read<ym_map>("year-month: value"sv);
		ASSERT_FALSE(value);
		ASSERT_EQ(log,
		          "input:1:1: error: expecting a number\n"
		          "input:1:1: error: expecting YYYY/MM or YYYY-MM\n"sv);

		value = read<ym_map>("2024-13: value"sv);
		ASSERT_FALSE(value);
		ASSERT_EQ(log, "input:1:1: error: expecting YYYY/MM or YYYY-MM\n"sv);

		// valid
		auto const expected_map = std::map{std::pair{2026y / 10, "value"s}};

		value = read<ym_map>("2026/10: value"sv);
		ASSERT_TRUE(value);
		ASSERT_EQ(*value, expected_map);
		ASSERT_EQ(log, empty_log);

		value = read<ym_map>("2026-10: value"sv);
		ASSERT_TRUE(value);
		ASSERT_EQ(*value, expected_map);
		ASSERT_EQ(log, empty_log);
	}

}  // namespace quick_dra::testing