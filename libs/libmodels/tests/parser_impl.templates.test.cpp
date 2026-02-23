// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "parser_impl.common.hpp"

namespace quick_dra::testing {
	class parser_impl : public ::testing::Test,
	                    public object_reader<templates> {};

	namespace debug {
		std::string format_field(templates_fields::value_type const& p) {
			auto const id = p.first;
			auto const& value = p.second;
			if (std::holds_alternative<std::string>(value)) {
				return fmt::format("{}: '{}'", id,
				                   std::get<std::string>(value));
			}

			return fmt::format(
			    "{}: [{}]", id,
			    fmt::join(std::get<std::vector<std::string>>(value), ", "));
		}

		std::string format_section_id(report_section const& section) {
			auto const block = section.block
			                       .transform([](auto const& block) {
				                       return fmt::format(".{}", block);
			                       })
			                       .value_or("");
			return fmt::format("{}{}", section.id, block);
		}

		std::string format_section(report_section const& section) {
			return fmt::format(
			    "{}=[{}]", format_section_id(section),
			    fmt::join(section.fields | std::views::transform(format_field),
			              ", "));
		}

		std::string format_report(templates_report const& report) {
			return fmt::to_string(fmt::join(
			    report | std::views::transform(format_section), ", "));
		}
	}  // namespace debug

	TEST_F(parser_impl, empty_template) {
		auto const value = read(R"()"sv);
		ASSERT_FALSE(value);
		ASSERT_EQ(
		    log,
		    R"(error: expecting `reports`; please use explicit {} to heave empty object instead
error: while reading `reports`
)"sv);
	}

	TEST_F(parser_impl, valid_template) {
		auto const value = read(R"(
version: 1
reports:
  CODE:
    - id: III
      block: A
      fields:
        1: $insured.last
        2: $insured.first
        3: $insured.document_kind
        4: $insured.document

    - id: ONE
      repeatable: true
      fields:
        1: 1
        2: 0zł
        3: 100zł
        4: $ref
        5: $+2,3
        6: [$refA, $refB]
)"sv);
		ASSERT_TRUE(value);
		ASSERT_TRUE(value->validate());
		ASSERT_EQ(log, R"()"sv);

		auto const& templates = *value;
		auto const expected = templates_reports{
		    {
		        "CODE"s,
		        templates_report{
		            report_section{
		                .id = "III"s,
		                .block = "A"s,
		                .repeatable = std::nullopt,
		                .fields =
		                    {
		                        {1u, "$insured.last"s},
		                        {2u, "$insured.first"s},
		                        {3u, "$insured.document_kind"s},
		                        {4u, "$insured.document"s},
		                    },
		            },
		            report_section{
		                .id = "ONE"s,
		                .block = std::nullopt,
		                .repeatable = true,
		                .fields =
		                    {
		                        {1u, "1"s},
		                        {2u, "0zł"s},
		                        {3u, "100zł"s},
		                        {4u, "$ref"s},
		                        {5u, "$+2,3"s},
		                        {6u, std::vector{"$refA"s, "$refB"s}},
		                    },
		            },
		        },
		    },
		};
		ASSERT_EQ(templates.reports, expected);
	}

	TEST_F(parser_impl, no_doc_id) {
		auto const value = read(R"(
version: 1
reports:
  '':
    - id: III
      block: A
      fields:
        1: $insured.last
        2: $insured.first
        3: $insured.document_kind
        4: $insured.document

    - id: ONE
      repeatable: true
      fields:
        1: 1
        2: 0zł
        3: 100zł
        4: $ref
        5: $+2,3
        6: [$refA, $refB]
)"sv);
		ASSERT_TRUE(value);
		ASSERT_FALSE(value->validate());
		ASSERT_EQ(log, R"()"sv);
	}

	TEST_F(parser_impl, no_block_id) {
		auto const value = read(R"(
version: 1
reports:
  CODE:
    - id: III
      block: ''
      fields:
        1: $insured.last
        2: $insured.first
        3: $insured.document_kind
        4: $insured.document

    - id: ONE
      repeatable: true
      fields:
        1: 1
        2: 0zł
        3: 100zł
        4: $ref
        5: $+2,3
        6: [$refA, $refB]
)"sv);
		ASSERT_TRUE(value);
		ASSERT_FALSE(value->validate());
		ASSERT_EQ(log, R"()"sv);
	}

	TEST_F(parser_impl, no_section_id) {
		auto const value = read(R"(
version: 1
reports:
  CODE:
    - id: ''
      block: A
      fields:
        1: $insured.last
        2: $insured.first
        3: $insured.document_kind
        4: $insured.document

    - id: ONE
      repeatable: true
      fields:
        1: 1
        2: 0zł
        3: 100zł
        4: $ref
        5: $+2,3
        6: [$refA, $refB]
)"sv);
		ASSERT_TRUE(value);
		ASSERT_FALSE(value->validate());
		ASSERT_EQ(log, R"()"sv);
	}

	TEST_F(parser_impl, no_field_value) {
		auto const value = read(R"(
version: 1
reports:
  CODE:
    - id: III
      block: A
      fields:
        1: $insured.last
        2: $insured.first
        3: ''
        4: $insured.document

    - id: ONE
      repeatable: true
      fields:
        1: 1
        2: 0zł
        3: 100zł
        4: $ref
        5: $+2,3
        6: [$refA, $refB]
)"sv);
		ASSERT_TRUE(value);
		ASSERT_FALSE(value->validate());
		ASSERT_EQ(log, R"()"sv);
	}

	TEST_F(parser_impl, no_field_value_in_list) {
		auto const value = read(R"(
version: 1
reports:
  CODE:
    - id: III
      block: A
      fields:
        1: $insured.last
        2: $insured.first
        3: $insured.document_kind
        4: $insured.document

    - id: ONE
      repeatable: true
      fields:
        1: 1
        2: 0zł
        3: 100zł
        4: $ref
        5: $+2,3
        6: ['', $refB]
)"sv);
		ASSERT_TRUE(value);
		ASSERT_FALSE(value->validate());
		ASSERT_EQ(log, R"()"sv);
	}
}  // namespace quick_dra::testing
