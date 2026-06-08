// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <gtest/gtest.h>
#include <app/utils/forms.hpp>
#include <app/utils/str.hpp>
#include <clocale>
#include <optional>
#include <print>
#include <span>
#include <string_view>

using namespace std::literals;

namespace quick_dra::gui::testing {
	std::optional<std::string> conv(std::optional<std::string_view> const& view) {
		if (view) return as_str(*view);
		return {};
	}

	struct document_info_testcase {
		std::optional<std::string_view> kind{};
		std::optional<std::string_view> document{};
		std::string_view expected{};

		std::pair<std::optional<std::string>, std::optional<std::string>> conv() const {
			return {testing::conv(kind), testing::conv(document)};
		}
	};

	struct salary_info_testcase {
		year_month month{null_month};
		std::optional<ratio> scale{};
		std::optional<currency> salary{};
		std::string_view expected{};
	};

	struct ratio_from_testcase {
		unsigned num{};
		unsigned den{};
		std::string_view expected{};
	};

	struct name_view {
		std::optional<std::string_view> first_name{};
		std::optional<std::string_view> last_name{};

		std::pair<std::optional<std::string>, std::optional<std::string>> conv() const {
			return {testing::conv(first_name), testing::conv(last_name)};
		}
	};

	static constexpr name_view missing_first{.last_name = "Last"sv};
	static constexpr name_view missing_last{.first_name = "First"sv};
	static constexpr name_view full_name{.first_name = "First"sv, .last_name = "Last"sv};

	struct name_from_testcase {
		name_view name{};
		unknown_name format{unknown_name::markdown};
		name_hint format_for{name_hint::insured};
		std::string_view expected{};
	};

	class document_info : public ::testing::TestWithParam<document_info_testcase> {};
	class salary_info : public ::testing::TestWithParam<salary_info_testcase> {};
	class ratio_from : public ::testing::TestWithParam<ratio_from_testcase> {};
	class name_from : public ::testing::TestWithParam<name_from_testcase> {};

	TEST_P(document_info, format) {
		auto const& [kind, document] = GetParam().conv();
		auto const& expected = GetParam().expected;
		auto const actual = gui::document_info(kind, document);
		EXPECT_EQ(actual, expected);
	}

	TEST(document_info, kinds) {
		EXPECT_EQ(gui::document_kind('R'), "REGON"sv);
		EXPECT_EQ(gui::document_kind('P'), "PESEL"sv);
		EXPECT_EQ(gui::document_kind('1'), "dowód osobisty"sv);
		EXPECT_EQ(gui::document_kind('2'), "paszport"sv);
		EXPECT_EQ(gui::document_kind('3'), "nieznany typ"sv);
		EXPECT_EQ(gui::document_kind('\0'), "nieznany typ"sv);
	}

	TEST(insurance_title_info, format) {
		ASSERT_EQ(gui::insurance_title_info({}), ""sv);
		ASSERT_EQ(gui::insurance_title_info(
		              insurance_title{.title_code = "ABCD"s, .pension_right = 120, .disability_level = 1500}),
		          "*tytuł:* ABCD 120 1500"sv);
	}

	TEST_P(salary_info, format) {
		std::setlocale(LC_ALL, "pl_PL.UTF-8");
		auto const& [month, scale, salary, expected] = GetParam();
		auto const actual = gui::salary_info(month, scale, salary);
		EXPECT_EQ(actual, expected);
	}

	TEST_P(ratio_from, format) {
		auto const& [num, den, expected] = GetParam();
		auto const actual = gui::ratio_from(num, den);
		EXPECT_EQ(actual, expected);
	}

	TEST_P(name_from, format) {
		auto const& [name, format, format_for, expected] = GetParam();
		auto const& [first_name, last_name] = name.conv();
		auto const actual = gui::name_from(first_name, last_name, {.format = format, .format_for = format_for});
		EXPECT_EQ(actual, expected);
	}

	TEST(second_line, no_arg) { EXPECT_EQ(gui::second_line(), ""sv); }
	TEST(second_line, only_empty) { EXPECT_EQ(gui::second_line(""sv, ""sv, ""sv), ""sv); }
	TEST(second_line, one_valid) { EXPECT_EQ(gui::second_line(""sv, ""sv, "property"sv, ""sv), "property"sv); }
	TEST(second_line, some_valid) {
		EXPECT_EQ(gui::second_line("one"sv, ""sv, "two"sv, ""s, "three"sv), "one, two, three"sv);
	}
	TEST(second_line, all_valid) { EXPECT_EQ(gui::second_line("one"sv, "two"s, "three"sv), "one, two, three"sv); }

	static constexpr document_info_testcase document_info_tests[] = {
	    {},
	    {"P"sv},
	    {{}, "ABC000XYZ"sv, "*nieznany typ:* ABC000XYZ"sv},
	    {"P"sv, "ABC000XYZ"sv, "*PESEL:* ABC000XYZ"sv},
	};

#define NBSP "\xC2\xA0"
#ifdef WIN32
#define GROUP_SEP NBSP
#else
#define GROUP_SEP "\xE2\x80\xAF"
#endif
#define FULL_SALARY "1" GROUP_SEP "234,56" NBSP "zł"
#define TWO_THIRDS "823,04" NBSP "zł"

	static constexpr salary_info_testcase salary_info_tests[] = {
	    {.expected = "*pensja:* minimalna"sv},
	    {.month = 2024y / 10, .expected = "*pensja:* minimalna (od 2024/10)"sv},
	    {.scale = ratio{3, 3}, .expected = "*pensja:* minimalna"sv},
	    {.month = 2024y / 10, .scale = ratio{3, 3}, .expected = "*pensja:* minimalna (od 2024/10)"sv},
	    {.scale = ratio{8, 12}, .expected = "*pensja:* ⅔ minimalnej"sv},
	    {.month = 2024y / 10, .scale = ratio{8, 12}, .expected = "*pensja:* ⅔ minimalnej (od 2024/10)"sv},
	    {.salary = 1234.56_PLN, .expected = "*pensja:* " FULL_SALARY ""sv},
	    {.month = 2024y / 10, .salary = 1234.56_PLN, .expected = "*pensja:* " FULL_SALARY " (od 2024/10)"sv},
	    {.scale = ratio{3, 3}, .salary = 1234.56_PLN, .expected = "*pensja:* " FULL_SALARY ""sv},
	    {.month = 2024y / 10,
	     .scale = ratio{3, 3},
	     .salary = 1234.56_PLN,
	     .expected = "*pensja:* " FULL_SALARY " (od 2024/10)"sv},
	    {.scale = ratio{8, 12}, .salary = 1234.56_PLN, .expected = "*pensja:* " TWO_THIRDS " (⅔ z " FULL_SALARY ")"sv},
	    {.month = 2024y / 10,
	     .scale = ratio{8, 12},
	     .salary = 1234.56_PLN,
	     .expected = "*pensja:* " TWO_THIRDS " (⅔ z " FULL_SALARY ", od 2024/10)"sv},
	};

	static constexpr ratio_from_testcase ratio_from_tests[] = {
	    {},
	    {.num = 1, .den = 10, .expected = "⅒"sv},
	    {.num = 2, .den = 10, .expected = "⅕"sv},
	    {.num = 3, .den = 10, .expected = "3/10"sv},
	    {.num = 4, .den = 10, .expected = "⅖"sv},
	    {.num = 5, .den = 10, .expected = "½"sv},
	    {.num = 6, .den = 10, .expected = "⅗"sv},
	    {.num = 7, .den = 10, .expected = "7/10"sv},
	    {.num = 8, .den = 10, .expected = "⅘"sv},
	    {.num = 9, .den = 10, .expected = "9/10"sv},
	    {.num = 10, .den = 10, .expected = "1/1"sv},
	    {.num = 1, .den = 9, .expected = "⅑"sv},
	    {.num = 2, .den = 9, .expected = "2/9"sv},
	    {.num = 3, .den = 9, .expected = "⅓"sv},
	    {.num = 4, .den = 9, .expected = "4/9"sv},
	    {.num = 5, .den = 9, .expected = "5/9"sv},
	    {.num = 6, .den = 9, .expected = "⅔"sv},
	    {.num = 7, .den = 9, .expected = "7/9"sv},
	    {.num = 8, .den = 9, .expected = "8/9"sv},
	    {.num = 9, .den = 9, .expected = "1/1"sv},
	};

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtrigraphs"
// warning: trigraph ??> ignored, use -trigraphs to enable [-Wtrigraphs]
#endif

	static constexpr name_from_testcase name_from_tests[] = {
	    {.expected = "*Nieznany ubezpieczony*"sv},
	    {.format = unknown_name::symbolic, .expected = "<Nieznany ubezpieczony>"sv},
	    {.format_for = name_hint::payer, .expected = "*Nieznany płatnik*"sv},
	    {.format = unknown_name::symbolic, .format_for = name_hint::payer, .expected = "<Nieznany płatnik>"sv},
	    {.name = missing_first, .expected = "*???* Last"sv},
	    {.name = missing_first, .format = unknown_name::symbolic, .expected = "<???> Last"sv},
	    {.name = missing_first, .format_for = name_hint::payer, .expected = "*???* Last"sv},
	    {.name = missing_first,
	     .format = unknown_name::symbolic,
	     .format_for = name_hint::payer,
	     .expected = "<???> Last"sv},
	    {.name = missing_last, .expected = "First *???*"sv},
	    {.name = missing_last, .format = unknown_name::symbolic, .expected = "First <???>"sv},
	    {.name = missing_last, .format_for = name_hint::payer, .expected = "First *???*"sv},
	    {.name = missing_last,
	     .format = unknown_name::symbolic,
	     .format_for = name_hint::payer,
	     .expected = "First <???>"sv},
	    {.name = full_name, .expected = "First Last"sv},
	    {.name = full_name, .format = unknown_name::symbolic, .expected = "First Last"sv},
	    {.name = full_name, .format_for = name_hint::payer, .expected = "First Last"sv},
	    {.name = full_name,
	     .format = unknown_name::symbolic,
	     .format_for = name_hint::payer,
	     .expected = "First Last"sv},
	};

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif  // __GNUC__

	INSTANTIATE_TEST_SUITE_P(tests, document_info, ::testing::ValuesIn(document_info_tests));
	INSTANTIATE_TEST_SUITE_P(tests, salary_info, ::testing::ValuesIn(salary_info_tests));
	INSTANTIATE_TEST_SUITE_P(tests, ratio_from, ::testing::ValuesIn(ratio_from_tests));
	INSTANTIATE_TEST_SUITE_P(tests, name_from, ::testing::ValuesIn(name_from_tests));
}  // namespace quick_dra::gui::testing
