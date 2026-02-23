// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "parser_impl.common.hpp"

namespace quick_dra::testing {
	using values = std::vector<calculated_value>;

	class compiler : public ::testing::Test, public object_reader<templates> {
	protected:
		std::vector<compiled_section> get_report_from_yaml(
		    std::string_view yaml) {
			auto const value = read(yaml);

			if (!value || !value->validate()) return {};
			return compiled_templates::compile(*value)
			    .reports.find("CODE"s)
			    ->second;
		}

		std::vector<compiled_section> get_report() {
			return get_report_from_yaml(R"(
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
        5: 1234zł
        6: $+1,2,3,4,5
        7: [$serial.A, $serial.B, $serial.C]
)"sv);
		}
	};

	TEST_F(compiler, valid_template) {
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
        5: 1234zł
        6: $+1,2,3,4,5
)"sv);
		ASSERT_TRUE(value);
		ASSERT_TRUE(value->validate());
		ASSERT_EQ(log, R"()"sv);

		auto const actual_template = compiled_templates::compile(*value);
		auto const expected_template = compiled_templates{
		    .reports = {{
		        "CODE"s,
		        std::vector<compiled_section>{
		            {
		                .id = "III"s,
		                .repeatable = false,
		                .blocks = {compiled_block{
		                    .id = "A"s,
		                    .fields =
		                        {
		                            {1u, "insured.last"_var},
		                            {2u, "insured.first"_var},
		                            {3u, "insured.document_kind"_var},
		                            {4u, "insured.document"_var},
		                            {5u, 1234_PLN},
		                            {6u,
		                             addition{.refs = {1u, 2u, 3u, 4u, 5u}}},
		                        },
		                }},
		            },
		        },
		    }},
		};

		ASSERT_EQ(actual_template, expected_template);
	}

	TEST_F(compiler, bad_currency) {
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
        5: many zł
        6: $+1,2,3,4,5
)"sv);
		ASSERT_TRUE(value);
		ASSERT_TRUE(value->validate());
		ASSERT_EQ(log, R"()"sv);

		auto const actual_template = compiled_templates::compile(*value);
		auto const expected_template = compiled_templates{
		    .reports = {{
		        "CODE"s,
		        std::vector<compiled_section>{
		            {
		                .id = "III"s,
		                .repeatable = false,
		                .blocks = {compiled_block{
		                    .id = "A"s,
		                    .fields =
		                        {
		                            {1u, "insured.last"_var},
		                            {2u, "insured.first"_var},
		                            {3u, "insured.document_kind"_var},
		                            {4u, "insured.document"_var},
		                            {5u, "unparsable: many zł"s},
		                            {6u,
		                             addition{.refs = {1u, 2u, 3u, 4u, 5u}}},
		                        },
		                }},
		            },
		        },
		    }},
		};

		ASSERT_EQ(actual_template, expected_template);
	}

	TEST_F(compiler, bad_addition_ref) {
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
        5: 1234zł
        6: $+1,2,50zł,4,5
)"sv);
		ASSERT_TRUE(value);
		ASSERT_TRUE(value->validate());
		ASSERT_EQ(log, R"()"sv);

		auto const actual_template = compiled_templates::compile(*value);
		auto const expected_template = compiled_templates{
		    .reports = {{
		        "CODE"s,
		        std::vector<compiled_section>{
		            {
		                .id = "III"s,
		                .repeatable = false,
		                .blocks = {compiled_block{
		                    .id = "A"s,
		                    .fields =
		                        {
		                            {1u, "insured.last"_var},
		                            {2u, "insured.first"_var},
		                            {3u, "insured.document_kind"_var},
		                            {4u, "insured.document"_var},
		                            {5u, 1234_PLN},
		                            {6u, "unparsable: $+1,2,50zł,4,5"s},
		                        },
		                }},
		            },
		        },
		    }},
		};

		ASSERT_EQ(actual_template, expected_template);
	}

	TEST_F(compiler, valid_report) {
		global_object globals{};

		globals.insert("insured.last"_var, 123_PLN);
		globals.insert("insured.first"_var, 223_PLN);
		globals.insert("insured.document_kind"_var, 323_PLN);
		globals.insert("insured.document"_var, 423_PLN);
		globals.insert("serial.A"_var, uint_value{99});
		globals.insert("serial.B"_var, "2016-01"s);
		globals.insert("serial.C"_var, 1.5_per);

		auto const compiled_report = get_report();
		auto const actual_report = calculate(compiled_report, globals);
		auto const expected_report = std::vector<calculated_section>{
		    {
		        .id = "III"s,
		        .repeatable = false,
		        .blocks = {calculated_block{
		            .id = "A"s,
		            .fields =
		                {
		                    {1u, 123_PLN},
		                    {2u, 223_PLN},
		                    {3u, 323_PLN},
		                    {4u, 423_PLN},
		                    {5u, 1234_PLN},
		                    {6u, 2326_PLN},
		                    {7u, values{uint_value{99}, "2016-01"s, 1.5_per}},
		                },
		        }},
		    },
		};
		ASSERT_EQ(actual_report, expected_report);
	}

	TEST_F(compiler, adding_string) {
		global_object globals{};

		globals.insert("insured.last"_var, 123_PLN);
		globals.insert("insured.first"_var, 223_PLN);
		globals.insert("insured.document_kind"_var, "P"s);
		globals.insert("insured.document"_var, 423_PLN);
		globals.insert("serial.A"_var, uint_value{99});
		globals.insert("serial.B"_var, "2016-01"s);
		globals.insert("serial.C"_var, 1.5_per);

		auto const compiled_report = get_report();
		auto const actual_report = calculate(compiled_report, globals);
		auto const expected_report = std::vector<calculated_section>{
		    {
		        .id = "III"s,
		        .repeatable = false,
		        .blocks = {calculated_block{
		            .id = "A"s,
		            .fields =
		                {
		                    {1u, 123_PLN},
		                    {2u, 223_PLN},
		                    {3u, "P"s},
		                    {4u, 423_PLN},
		                    {5u, 1234_PLN},
		                    {6u, {}},
		                    {7u, values{uint_value{99}, "2016-01"s, 1.5_per}},
		                },
		        }},
		    },
		};
		ASSERT_EQ(actual_report, expected_report);
	}

	TEST_F(compiler, adding_list) {
		global_object globals{};

		globals.insert("insured.last"_var, 123_PLN);
		globals.insert("insured.first"_var, 223_PLN);
		globals.insert("insured.document_kind"_var, 323_PLN);
		globals.insert("insured.document"_var,
		               values{15_PLN, uint_value{14}, "WORD"s});
		globals.insert("serial.A"_var, uint_value{99});
		globals.insert("serial.B"_var, "2016-01"s);
		globals.insert("serial.C"_var, 1.5_per);

		auto const compiled_report = get_report();
		auto const actual_report = calculate(compiled_report, globals);
		auto const expected_report = std::vector<calculated_section>{
		    {
		        .id = "III"s,
		        .repeatable = false,
		        .blocks = {calculated_block{
		            .id = "A"s,
		            .fields =
		                {
		                    {1u, 123_PLN},
		                    {2u, 223_PLN},
		                    {3u, 323_PLN},
		                    {
		                        4u,
		                        values{15_PLN, uint_value{14}, "WORD"s},
		                    },
		                    {5u, 1234_PLN},
		                    {6u, {}},
		                    {7u, values{uint_value{99}, "2016-01"s, 1.5_per}},
		                },
		        }},
		    },
		};
		ASSERT_EQ(actual_report, expected_report);
	}

	TEST_F(compiler, dereferencing_null) {
		global_object globals{};

		globals.insert("insured.last"_var, 123_PLN);
		globals.get("insured.first"_var);
		globals.insert("insured.document_kind"_var, 323_PLN);
		globals.insert("insured.document"_var, 423_PLN);
		globals.insert("serial.A"_var, uint_value{99});
		globals.insert("serial.B"_var, "2016-01"s);
		globals.insert("serial.C"_var, 1.5_per);

		auto const compiled_report = get_report();
		auto const actual_report = calculate(compiled_report, globals);
		auto const expected_report = std::vector<calculated_section>{
		    {
		        .id = "III"s,
		        .repeatable = false,
		        .blocks = {calculated_block{
		            .id = "A"s,
		            .fields =
		                {
		                    {1u, 123_PLN},
		                    {2u, {}},
		                    {3u, 323_PLN},
		                    {4u, 423_PLN},
		                    {5u, 1234_PLN},
		                    {6u, {}},
		                    {7u, values{uint_value{99}, "2016-01"s, 1.5_per}},
		                },
		        }},
		    },
		};
		ASSERT_EQ(actual_report, expected_report);
	}

	TEST_F(compiler, dereferencing_list) {
		global_object globals{};

		globals.insert("insured.last"_var, 123_PLN);
		globals.insert("insured.first"_var, 223_PLN);
		globals.insert("insured.document_kind"_var, 323_PLN);
		globals.insert("insured.document"_var, 423_PLN);
		globals.insert("serial.A"_var, uint_value{99});
		globals.insert("serial.B"_var, values{300_PLN, 0_PLN});
		globals.insert("serial.C"_var, 1.5_per);

		auto const compiled_report = get_report();
		auto const actual_report = calculate(compiled_report, globals);
		auto const expected_report = std::vector<calculated_section>{
		    {
		        .id = "III"s,
		        .repeatable = false,
		        .blocks = {calculated_block{
		            .id = "A"s,
		            .fields =
		                {
		                    {1u, 123_PLN},
		                    {2u, 223_PLN},
		                    {3u, 323_PLN},
		                    {4u, 423_PLN},
		                    {5u, 1234_PLN},
		                    {6u, 2326_PLN},
		                    {7u, values{uint_value{99}, {}, 1.5_per}},
		                },
		        }},
		    },
		};
		ASSERT_EQ(actual_report, expected_report);
	}

	TEST_F(compiler, adding_a_missing_ref) {
		global_object globals{};

		globals.insert("insured.last"_var, 123_PLN);
		globals.insert("insured.first"_var, 223_PLN);
		globals.insert("insured.document_kind"_var, 323_PLN);
		globals.insert("insured.document"_var, 423_PLN);
		globals.insert("serial.A"_var, uint_value{99});
		globals.insert("serial.B"_var, "2016-01"s);
		globals.insert("serial.C"_var, 1.5_per);

		auto const compiled_report = get_report_from_yaml(R"(
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
        5: 1234zł
        6: $+1,2,3,4,5,8
        7: [$serial.A, $serial.B, $serial.C]
)"sv);

		auto const actual_report = calculate(compiled_report, globals);
		auto const expected_report = std::vector<calculated_section>{
		    {
		        .id = "III"s,
		        .repeatable = false,
		        .blocks = {calculated_block{
		            .id = "A"s,
		            .fields =
		                {
		                    {1u, 123_PLN},
		                    {2u, 223_PLN},
		                    {3u, 323_PLN},
		                    {4u, 423_PLN},
		                    {5u, 1234_PLN},
		                    {6u, {}},
		                    {7u, values{uint_value{99}, "2016-01"s, 1.5_per}},
		                },
		        }},
		    },
		};

		ASSERT_EQ(actual_report, expected_report);
	}

	TEST_F(compiler, adding_inside_a_list) {
		global_object globals{};

		globals.insert("insured.last"_var, 123_PLN);
		globals.insert("insured.first"_var, 223_PLN);
		globals.insert("insured.document_kind"_var, 323_PLN);
		globals.insert("insured.document"_var, 423_PLN);
		globals.insert("serial.A"_var, uint_value{99});
		globals.insert("serial.B"_var, "2016-01"s);
		globals.insert("serial.C"_var, 1.5_per);

		auto const compiled_report = get_report_from_yaml(R"(
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
        5: 1234zł
        6: $+1,2,3,4,5
        7: [$serial.A, "$+1,5", $serial.C]
)"sv);

		auto const actual_report = calculate(compiled_report, globals);
		auto const expected_report = std::vector<calculated_section>{
		    {
		        .id = "III"s,
		        .repeatable = false,
		        .blocks = {calculated_block{
		            .id = "A"s,
		            .fields =
		                {
		                    {1u, 123_PLN},
		                    {2u, 223_PLN},
		                    {3u, 323_PLN},
		                    {4u, 423_PLN},
		                    {5u, 1234_PLN},
		                    {6u, 2326_PLN},
		                    {7u, values{uint_value{99}, {}, 1.5_per}},
		                },
		        }},
		    },
		};
		ASSERT_EQ(actual_report, expected_report);
	}
}  // namespace quick_dra::testing
