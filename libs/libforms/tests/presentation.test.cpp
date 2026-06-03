// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <chrono>
#include <quick_dra/docs/presentation.hpp>
#include <string>
#include <string_view>

#define NBSP "\xC2\xA0"

using namespace std::literals;
using namespace std::chrono;

namespace quick_dra::testing {
	static constexpr auto good_format = R"(titles:
 DOC-TYPE: [SEG1.3, "= ", SEG1.4, "=! what?, yeah: ", SEG2.B.5]

format hints:
 DOC-TYPE:
  SEG1.1: left
  SEG1.2: {align: '<'}
  SEG1.3: {align: right}
  SEG1.4: {ignore: false, align: '>'}
  SEG1.5: {ignore: true, sep: ' | '}
  SEG1.7: {sep: ' | '}
  SEG2.A: {ignore: true}
  SEG2.B.7: {sep: '/'}
 ONLY-FORMATS:
  IV.B.2: right

labels:
 DOC-TYPE:
  SEG1:
   label: Segment #1
   fields:
    1: Field #1
    2: Field #2
    7: Field #7
 ONLY-LABELS:
  I:
    label: This will go through adding a document to result at "labels" stage
)"sv;

#define TEST_DEFAULT_ALIGNMENT_(NAME, TYPE, ALIGNMENT)                            \
	TEST(report_format, default_alignment_##NAME) {                               \
		report_format::hint hint{};                                               \
		EXPECT_EQ(report_format::hint::default_alignment_for(TYPE{}), ALIGNMENT); \
		/* used, when not overriden */                                            \
		EXPECT_EQ(hint.alignment_for(TYPE{}), ALIGNMENT);                         \
	}

#define TEST_DEFAULT_ALIGNMENT(TYPE, ALIGNMENT) TEST_DEFAULT_ALIGNMENT_(TYPE, TYPE, ALIGNMENT)

	TEST_DEFAULT_ALIGNMENT(percent, alignement::right);
	TEST_DEFAULT_ALIGNMENT(currency, alignement::right);
	TEST_DEFAULT_ALIGNMENT(uint_value, alignement::right);
	TEST_DEFAULT_ALIGNMENT_(year_month, std::chrono::year_month, alignement::right);
	TEST_DEFAULT_ALIGNMENT_(year_month_day, std::chrono::year_month_day, alignement::right);
	TEST_DEFAULT_ALIGNMENT_(string, std::string, alignement::left);
	TEST_DEFAULT_ALIGNMENT_(monostate, std::monostate, alignement::left);

	TEST(report_format, default_alignment_calculated_value) {
		EXPECT_EQ(report_format::hint::default_alignment_for(calculated_value{}), alignement::left);
		EXPECT_EQ(report_format::hint::default_alignment_for(calculated_value{"string"s}), alignement::left);
		EXPECT_EQ(report_format::hint::default_alignment_for(calculated_value{50_per}), alignement::right);
		EXPECT_EQ(report_format::hint::default_alignment_for(calculated_value{1_PLN}), alignement::right);
		EXPECT_EQ(report_format::hint::default_alignment_for(calculated_value{2026y / June}), alignement::right);
		EXPECT_EQ(report_format::hint::default_alignment_for(calculated_value{2026y / June / 15}), alignement::right);
		EXPECT_EQ(report_format::hint::default_alignment_for(calculated_value{uint_value{42}}), alignement::right);
	}

	TEST(report_format, alignement_overriden_by_hint) {
		report_format::hint hint_left{.alignment_ = alignement::left};
		report_format::hint hint_right{.alignment_ = alignement::right};

		EXPECT_EQ(hint_left.alignment_for(30_per), alignement::left);
		EXPECT_EQ(hint_left.alignment_for(1000_PLN), alignement::left);
		EXPECT_EQ(hint_left.alignment_for(2024y / 10 / 31), alignement::left);
		EXPECT_EQ(hint_right.alignment_for("tysiąc pięćset sto dziewięćset"s), alignement::right);
	}

	TEST(report_format, ignore_with_inheritance) {
		report_format::hint hint_right{.alignment_ = alignement::right};

		EXPECT_EQ(report_format::hint{.ignore_ = true}.ignore(true), true);
		EXPECT_EQ(report_format::hint{.ignore_ = true}.ignore(false), true);
		EXPECT_EQ(report_format::hint{.ignore_ = false}.ignore(true), false);
		EXPECT_EQ(report_format::hint{.ignore_ = false}.ignore(false), false);
		EXPECT_EQ(report_format::hint{}.ignore(true), true);
		EXPECT_EQ(report_format::hint{}.ignore(false), false);
	}

	TEST(report_format, load_format) {
		auto const actual = report_format::formatting::parse(as_str(good_format), "file name"s);
		ASSERT_EQ(actual.size(), 3);
		ASSERT_NE(actual.find("DOC-TYPE"s), actual.end());

		auto const& doc_type = actual.at("DOC-TYPE"s);

		std::vector<report_format::ref> title{
		    {.section = "SEG1"s, .field = 3},
		    {.section = " "s, .field = 0},
		    {.section = "SEG1", .field = 4},
		    {.section = "! what?, yeah: "s, .field = 0},
		    {.section = "SEG2"s, .block = "B"s, .field = 5},
		};

		ASSERT_EQ(doc_type.title.size(), title.size());
		auto title_it = title.begin();
		for (auto const& act : doc_type.title) {
			auto const& exp = *title_it++;
			ASSERT_EQ(act.section, exp.section);
			ASSERT_EQ(act.block, exp.block);
			ASSERT_EQ(act.field, exp.field);
		}

		std::map<std::string, report_format::hint> hints{
		    {"SEG1.1"s, {.alignment_ = alignement::left}},
		    {"SEG1.2"s, {.alignment_ = alignement::left}},
		    {"SEG1.3"s, {.alignment_ = alignement::right}},
		    {"SEG1.4"s, {.ignore_ = false, .alignment_ = alignement::right}},
		    {"SEG1.5"s, {.sep = " | "s, .ignore_ = true}},
		    {"SEG1.7"s, {.sep = " | "s}},
		    {"SEG2.A"s, {.ignore_ = true}},
		    {"SEG2.B.7"s, {.sep = "/"s}},
		};
		ASSERT_EQ(doc_type.hints.size(), hints.size());
		for (auto const& [key, exp] : hints) {
			auto it = doc_type.hints.find(key);
			ASSERT_NE(it, doc_type.hints.end());
			auto const& act = it->second;
			ASSERT_EQ(act.sep, exp.sep);
			ASSERT_EQ(act.ignore_, exp.ignore_);
			ASSERT_EQ(act.alignment_, exp.alignment_);
		}
	}

	TEST(report_format, load_format_errors_title_ref) {
		::testing::internal::CaptureStderr();
		ASSERT_EQ(report_format::formatting::parse("titles: {D: [SECTION]}"s, "file name"s).size(), 0);
		ASSERT_EQ(::testing::internal::GetCapturedStderr(),
		          "file name:1:14: error: expected SECTION[.BLOCK].FIELD, got `SECTION`\n"
		          "file name:1:1: error: while reading `titles'\n"
		          ""sv);
		::testing::internal::CaptureStderr();
		ASSERT_EQ(report_format::formatting::parse("titles: {D: [SEC.BLOCK.3.SOMETHING]}"s, "file name"s).size(), 0);
		ASSERT_EQ(::testing::internal::GetCapturedStderr(),
		          "file name:1:14: error: expected SECTION[.BLOCK].FIELD, got `SEC.BLOCK.3.SOMETHING`\n"
		          "file name:1:1: error: while reading `titles'\n"
		          ""sv);
		::testing::internal::CaptureStderr();
		ASSERT_EQ(report_format::formatting::parse("titles: {D: [SEC.BLOCK.SMTH]}"s, "file name"s).size(), 0);
		ASSERT_EQ(::testing::internal::GetCapturedStderr(),
		          "file name:1:14: error: expecting a positive number\n"
		          "file name:1:1: error: while reading `titles'\n"
		          ""sv);
	}

	TEST(report_format, load_format_errors_alignment) {
		::testing::internal::CaptureStderr();
		auto const actual = report_format::formatting::parse("format hints: {D: {FLD.REF: justify}}"s, "file name"s);
		auto err = ::testing::internal::GetCapturedStderr();
		ASSERT_EQ(err,
		          "file name:1:20: error: unknown value, `justify`\n"
		          "file name:1:1: error: while reading `format hints'\n"
		          ""sv);
		ASSERT_EQ(actual.size(), 0);
	}

	TEST(report_format, add_new_section) {
		formatted_report report;
		std::map<std::string, std::string> labels{
		    {"income"s, "Income"s},
		};

		data_field field{
		    .number = 1,
		    .formatted = "5000"s,
		    .label = "Salary"s,
		    .alignement = alignement::right,
		};

		report.add("income"s, field, labels);

		EXPECT_EQ(report.data.size(), 1);
		EXPECT_EQ(report.order.size(), 1);
		EXPECT_EQ(report.order[0], "income"s);
		EXPECT_TRUE(report.data.count("income"s));
		EXPECT_EQ(report.data.at("income"s).name, "income"s);
		EXPECT_EQ(report.data.at("income"s).label, "Income"s);
		EXPECT_EQ(report.data.at("income"s).fields.size(), 1);
	}

	TEST(report_format, add_to_existing_section) {
		formatted_report report;
		std::map<std::string, std::string> labels{
		    {"income"s, "Income"s},
		};

		data_field field1{
		    .number = 1,
		    .formatted = "5000"s,
		    .label = "Salary"s,
		    .alignement = alignement::right,
		};
		data_field field2{
		    .number = 2,
		    .formatted = "500"s,
		    .label = "Bonus"s,
		    .alignement = alignement::right,
		};

		report.add("income"s, field1, labels);
		report.add("income"s, field2, labels);

		EXPECT_EQ(report.data.size(), 1);
		EXPECT_EQ(report.order.size(), 1);
		EXPECT_EQ(report.data.at("income"s).fields.size(), 2);
		EXPECT_EQ(report.data.at("income"s).fields[0].number, 1);
		EXPECT_EQ(report.data.at("income"s).fields[1].number, 2);
	}

	TEST(report_format, format_doc) {
		auto const mapping = report_format::formatting::parse(as_str(good_format), "file name"s);
		std::vector<calculated_section> form{
		    {
		        .id = "SEG1"s,
		        .blocks{{.fields{
		            {1, 2024y / April / 25},
		            {2, 2026y / October},
		            {3, "title1"s},  // title
		            {4, "title2"s},  // title
		            {5, 2200_PLN},   // sep, ignored
		            {6, 1.67_per},
		            {7, std::vector<calculated_value>{3.15_PLN, "word"s, 15_per}},  // sep
		            {8, 10'000_PLN},
		        }}},
		    },
		    {
		        .id = "SEG2"s,
		        .blocks{
		            {.id = "A"s,
		             .fields{
		                 {1, 0_PLN},
		                 {2, 0_PLN},
		                 {3, 0_PLN},
		             }},
		            {.id = "B"s,
		             .fields{
		                 {1, 123_PLN},
		                 {2, 23.45_per},
		                 {3, "words"s},
		                 {4, 1998y / December},
		                 {5, uint_value{42}},  // title
		                 {6, 1997y / May / 3},
		                 {7, std::vector<calculated_value>{3.15_PLN, 15_per, uint_value{42}}},
		                 {8, {}},
		             }},
		        },
		    },
		};
		auto const actual = report_format::formatting::format_report(mapping, "DOC-TYPE"s, form);
		std::vector<data_field> fields_SEG1{
		    {.number = 1, .formatted = "25-04-2024"s, .label = "Field"s},
		    {.number = 2, .formatted = "10-2026"s, .label = "Field"s},
		    {.number = 3, .formatted = "title1"s, .alignement = alignement::right},
		    {.number = 4, .formatted = "title2"s, .alignement = alignement::right},
		    {.number = 6, .formatted = "1,67%"s, .alignement = alignement::right},
		    {.number = 7, .formatted = "3,15" NBSP "zł | word | 15,00%"s, .label = "Field"s},
		    {.number = 8, .formatted = "10" NBSP "000,00" NBSP "zł"s, .alignement = alignement::right},
		};
		std::vector<data_field> fields_SEG2_B{
		    {.number = 1, .formatted = "123,00" NBSP "zł"s, .alignement = alignement::right},
		    {.number = 2, .formatted = "23,45%"s, .alignement = alignement::right},
		    {.number = 3, .formatted = "words"s},
		    {.number = 4, .formatted = "12-1998"s, .alignement = alignement::right},
		    {.number = 5, .formatted = "42"s, .alignement = alignement::right},
		    {.number = 6, .formatted = "03-05-1997"s, .alignement = alignement::right},
		    {.number = 7, .formatted = "3,15" NBSP "zł/15,00%/42"s},
		    {.number = 8},
		};

		EXPECT_EQ(actual.title, "DOC-TYPE (title1 title2! what?, yeah: 42)"sv);
		EXPECT_EQ(actual.order, (std::vector{"SEG1"s, "SEG2.B"s}));

		auto const& seg1 = actual.data.at("SEG1"s);
		auto const& seg2 = actual.data.at("SEG2.B"s);

		EXPECT_EQ(seg1.name, "SEG1"s);
		EXPECT_EQ(seg1.label, "Segment"s);
		ASSERT_EQ(seg1.fields.size(), fields_SEG1.size());
		auto it = fields_SEG1.begin();
		for (auto const& act : seg1.fields) {
			auto const& exp = *it++;
			EXPECT_EQ(act.number, exp.number);
			EXPECT_EQ(act.formatted, exp.formatted);
			EXPECT_EQ(act.label, exp.label);
			EXPECT_EQ(act.alignement, exp.alignement);
		}

		EXPECT_EQ(seg2.name, "SEG2. B"s);
		EXPECT_EQ(seg2.label, ""s);
		ASSERT_EQ(seg2.fields.size(), fields_SEG2_B.size());
		it = fields_SEG2_B.begin();
		for (auto const& act : seg2.fields) {
			auto const& exp = *it++;
			EXPECT_EQ(act.number, exp.number);
			EXPECT_EQ(act.formatted, exp.formatted);
			EXPECT_EQ(act.label, exp.label);
			EXPECT_EQ(act.alignement, exp.alignement);
		}
	}

	TEST(report_format, bad_title_missing_value_undefined_value_and_list_of_values) {
		auto const mapping =
		    report_format::formatting::parse("titles: {D: [I.1, II.1, II.2, '=/', II.3, '=, ', III.1]}", "file name"s);
		std::vector<calculated_section> form{
		    {.id = "I"s, .blocks{{.fields{{2, 2026y / October}}}}},  // I.1 is missing
		    {.id = "II"s,
		     .blocks{
		         {.fields{
		             {1, 0_PLN},                                                       // II.1 is present
		             {2, {}},                                                          // II.2 is monostate/undefined
		             {3, std::vector<calculated_value>{3_PLN, 4_per, uint_value{5}}},  // II.2 is a list
		         }},
		     }},
		    // III is missing
		};
		auto const actual = report_format::formatting::format_report(mapping, "D"s, form);

		EXPECT_EQ(actual.title, "D (?0,00" NBSP "zł/?, ?)"sv);
		EXPECT_EQ(actual.order, (std::vector{"I"s, "II"s}));
	}
}  // namespace quick_dra::testing
