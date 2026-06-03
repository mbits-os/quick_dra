// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <quick_dra/docs/locale.hpp>
#include <string>
#include <string_view>

using namespace std::literals;

#define NBSP "\xC2\xA0"

namespace quick_dra::locale::testing {
	struct decorator_case {
		bool was_negative = true;
		bool cs_precedes = false;
		bool sep_by_space = false;
		int sign_pos = 1;
		std::string_view expected{};

		void run() const {
			auto const actual = symbol_decorator::number_view::from("±"sv, cs_precedes, sep_by_space, sign_pos)
			                        .format("¤"sv, " <-> "sv, "{num}"sv, was_negative);
			ASSERT_EQ(actual, expected);
		}
	};

	struct grouping_case {
		number_grouping grouping{};
		std::string_view number{};
		std::string_view fraction{};
		std::string_view expected{};

		void run() const {
			auto const actual = grouping.group(number, fraction);
			ASSERT_EQ(actual, expected);
		}
	};

	static constexpr number_grouping test_splitter{
	    .decimal_point = ";;"sv,
	    .group_separator = "`"sv,
	    .grouping = "\003\002\002\003"sv,
	};

	static constexpr decorator_case decorator_tests[] = {
	    {.was_negative = false, .sign_pos = 0, .expected = "{num}¤"sv},
	    {.sign_pos = 0, .expected = "({num}¤)"sv},
	    {.sign_pos = 1, .expected = "±{num}¤"sv},
	    {.sign_pos = 2, .expected = "{num}¤±"sv},
	    {.sign_pos = 3, .expected = "{num}±¤"sv},
	    {.sign_pos = 4, .expected = "{num}¤±"sv},
	    {.sep_by_space = true, .sign_pos = 0, .expected = "({num} <-> ¤)"sv},
	    {.sep_by_space = true, .sign_pos = 1, .expected = "±{num} <-> ¤"sv},
	    {.sep_by_space = true, .sign_pos = 2, .expected = "{num} <-> ¤±"sv},
	    {.sep_by_space = true, .sign_pos = 3, .expected = "{num} <-> ±¤"sv},
	    {.sep_by_space = true, .sign_pos = 4, .expected = "{num} <-> ¤±"sv},
	    {.cs_precedes = true, .sign_pos = 0, .expected = "(¤{num})"sv},
	    {.cs_precedes = true, .sign_pos = 1, .expected = "±¤{num}"sv},
	    {.cs_precedes = true, .sign_pos = 2, .expected = "¤{num}±"sv},
	    {.cs_precedes = true, .sign_pos = 3, .expected = "±¤{num}"sv},
	    {.cs_precedes = true, .sign_pos = 4, .expected = "¤±{num}"sv},
	    {.cs_precedes = true, .sep_by_space = true, .sign_pos = 0, .expected = "(¤ <-> {num})"sv},
	    {.cs_precedes = true, .sep_by_space = true, .sign_pos = 1, .expected = "±¤ <-> {num}"sv},
	    {.cs_precedes = true, .sep_by_space = true, .sign_pos = 2, .expected = "¤ <-> {num}±"sv},
	    {.cs_precedes = true, .sep_by_space = true, .sign_pos = 3, .expected = "±¤ <-> {num}"sv},
	    {.cs_precedes = true, .sep_by_space = true, .sign_pos = 4, .expected = "¤± <-> {num}"sv},
	};

	TEST(locale, monetary_decorator) {
		for (auto const& test : decorator_tests) {
			test.run();
			if (::testing::Test::HasFailure()) {
				fmt::print("{} | {} | {}\n", test.cs_precedes ? "¤ < {num}" : "{num} > ¤",
				           test.sep_by_space ? "S" : "-", test.sign_pos);
			}
			if (::testing::Test::HasFatalFailure()) break;
		}
	}

	static constexpr grouping_case grouping_tests[] = {
	    {.grouping = test_splitter, .number = "0"sv, .expected = "0"sv},
	    {.grouping = test_splitter, .number = "123"sv, .fraction = "45678"sv, .expected = "123;;45678"sv},
	    {
	        .grouping = test_splitter,
	        .number = "123456789012345678901234567890"sv,
	        .expected = "12`345`678`901`234`567`890`123`45`67`890"sv,
	    },
	    {
	        .grouping = test_splitter,
	        .number = "901234567890"sv,
	        .fraction = "45"sv,
	        .expected = "90`123`45`67`890;;45"sv,
	    },
	};

	TEST(locale, number_grouping) {
		for (auto const& test : grouping_tests) {
			test.run();
			if (::testing::Test::HasFatalFailure()) break;
		}
	}

	TEST(locale, number_grouping_CHAR_MAX) {
		auto groups_str = "\003\002\002X\003"s;
		groups_str[groups_str.find('X')] = number_grouping::stop_repeating_blocks;

		number_grouping grouping_char_max{
		    .decimal_point = ","sv,
		    .group_separator = "`"sv,
		    .grouping = groups_str,
		};

		number_grouping grouping_empty{
		    .decimal_point = ","sv,
		    .group_separator = "`"sv,
		    .grouping = ""sv,
		};

		ASSERT_EQ(grouping_char_max.group("123456789012345678901234567890"sv, "2345"sv),
		          "12345678901234567890123`45`67`890,2345"sv);
		ASSERT_EQ(grouping_empty.group("123456789012345678901234567890"sv, "2345"sv),
		          "123456789012345678901234567890,2345"sv);
	}

	TEST(locale, system) {
		std::setlocale(LC_ALL, "en_US.UTF-8");
		auto const formatter = symbol_decorator::from_locale();
		auto const mon = number_grouping::monetary_from_locale();
		auto const num = number_grouping::numeric_from_locale();

		EXPECT_EQ(formatter.format(mon.group("123456"sv), false), "$123,456");
		EXPECT_EQ(formatter.format(mon.group("1456"sv, "23"sv), true), "-$1,456.23");
		EXPECT_EQ(num.group("1456"sv), "1,456");

		auto format = formatter::monetary_from_locale();
		EXPECT_EQ(format.format(-12345, 100), "-$123.45");
		EXPECT_EQ(format.format(-32012045, 1000), "-$32,012.045");
		EXPECT_EQ(format.format(12345), "$12,345");
		EXPECT_EQ(format.format(12345, 100), "$123.45");
		EXPECT_EQ(format.format(32012045, 1000), "$32,012.045");

		format = formatter::PLN_from_locale(" "sv);
		EXPECT_EQ(format.format(-12345, 100), "-123.45 zł");
		EXPECT_EQ(format.format(-32012045, 1000), "-32,012.045 zł");
		EXPECT_EQ(format.format(12345), "12,345 zł");
		EXPECT_EQ(format.format(12345, 100), "123.45 zł");
		EXPECT_EQ(format.format(32012045, 1000), "32,012.045 zł");

		EXPECT_EQ(format.format((3500_PLN).calc() * 106.33_per), "3,721.5500 zł");
		EXPECT_EQ(format.format(3675_PLN), "3,675.00 zł");
		EXPECT_EQ(format.grouping.group(3675.46_PLN), "3,675.46");

		format = formatter::percent_from_locale();
		EXPECT_EQ(format.format(-12345, 100), "-123.45%");
		EXPECT_EQ(format.format(-32012045, 1000), "-32,012.045%");
		EXPECT_EQ(format.format(12345), "12,345%");
		EXPECT_EQ(format.format(12345, 100), "123.45%");
		EXPECT_EQ(format.format(32012045, 1000), "32,012.045%");
		EXPECT_EQ(format.format(106.33_per), "106.33%");
		EXPECT_EQ(format.grouping.group(106.33_per), "106.33");

		EXPECT_EQ(from_system((3500_PLN).calc() * 106.33_per), "3,721.5500" NBSP "zł");
		EXPECT_EQ(from_system(3675_PLN, " : "), "3,675.00 : zł");
		EXPECT_EQ(from_system(106.33_per), "106.33%");
	}

	TEST(locale, C) {
		std::setlocale(LC_ALL, "C");
		EXPECT_EQ(from_system((3500_PLN).calc() * 106.33_per), "3721.5500" NBSP "zł");
		EXPECT_EQ(from_system((3675_PLN)), "3675.00" NBSP "zł");
		EXPECT_EQ(from_system(106.33_per), "106.33%");
	}
}  // namespace quick_dra::locale::testing
