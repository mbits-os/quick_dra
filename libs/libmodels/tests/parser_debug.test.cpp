// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <array>
#include <map>
#include <quick_dra/models/project_reader.hpp>
#include <ranges>
#include <span>
#include <utility>
#include <variant>
#include <vector>
#include <yaml/parser.hpp>

namespace quick_dra::testing {
	std::string to_str(verbose level) {
		switch (level) {
			case verbose::none:
				return "verbose::none"s;
			case verbose::names_only:
				return "verbose::names_only"s;
			case verbose::names_and_summary:
				return "verbose::names_and_summary"s;
			case verbose::names_and_details:
				return "verbose::names_and_details"s;
			case verbose::parameters:
				return "verbose::parameters"s;
			case verbose::raw_form_data:
				return "verbose::raw_form_data"s;
			case verbose::templates:
				return "verbose::templates"s;
			case verbose::calculated_sections:
				return "verbose::calculated_sections"s;
			case verbose::last:
				return "verbose::last"s;
		}
		return fmt::format("verbose{{0x{:x}}}", std::to_underlying(level));
	};

	static constexpr auto verbose_levels = std::array{
	    verbose::none,
	    verbose::names_only,
	    verbose::names_and_summary,
	    verbose::names_and_details,
	    verbose::parameters,
	    verbose::raw_form_data,
	    verbose::templates,
	    verbose::calculated_sections,
	    verbose::last,
	};

	struct read_print_testcase {
		std::string_view yaml{};
		void (*debug_print)(std::string_view, verbose) = nullptr;
		std::array<std::string_view, std::to_underlying(verbose::last) + 1>
		    expected{};

		template <verbose... Level>
		[[nodiscard]] consteval read_print_testcase on(
		    std::string_view output) const noexcept {
			auto copy = *this;
			((copy.expected[std::to_underlying(Level)] = output), ...);
			return copy;
		}

		[[nodiscard]] std::string run_at(verbose level) const {
			::testing::internal::CaptureStdout();
			(*debug_print)(yaml, level);
			return ::testing::internal::GetCapturedStdout();
		}
	};

	class parser_debug : public ::testing::TestWithParam<read_print_testcase> {
	public:
	};

	TEST_P(parser_debug, format) {
		auto const& test = GetParam();
		for (auto const level : verbose_levels) {
			auto const actual = test.run_at(level);
			auto const expected = test.expected[std::to_underlying(level)];
			ASSERT_EQ(actual, expected) << "Debug level: " << to_str(level);
		}
	}

	auto& postproc_load(auto& obj) { return obj; }  //-V669

	config& postproc_load(config& cfg) {
		cfg.params.scale = {
		    {30'000_PLN, 17_per},
		    {120'000_PLN, 32_per},
		};
		cfg.params.minimal_pay = 4'800_PLN;
		cfg.params.costs_of_obtaining = {.local = 250_PLN, .remote = 300_PLN};
		cfg.params.contributions = {
		    .health_insurance = {.payer = 9.76_per, .insured = 9.76_per},
		    .pension_insurance = {.payer = 6.5_per, .insured = 1.5_per},
		    .disability_insurance = {},  // force "payer 0%" to debug
		    .accident_insurance = {.payer = 1.67_per},
		    .health = {.insured = 9_per},
		};
		return cfg;
	}

	struct compiled_templates_helper {
		compiled_templates templates{};
		void debug_print(verbose level) const noexcept {
			if (level == verbose::templates) {
				templates.debug_print();
			}
		}
	};

	compiled_templates_helper postproc_load(templates& src) {
		return {.templates = compiled_templates::compile(src)};
	}

	template <typename FileObj>
	void load_and_print(std::string_view text, verbose level) {
		auto object = parser::parse_yaml_text<FileObj>(
		    {text.data(), text.size()}, "input"s);
		if (!object) return;
		auto const& obj = postproc_load(static_cast<FileObj&>(*object));
		obj.debug_print(level);
	}

	template <typename FileObj>
	inline consteval read_print_testcase test(std::string_view yaml) {
		return {
		    .yaml{yaml},
		    .debug_print{load_and_print<FileObj>},
		    .expected{},
		};
	};

	static constexpr read_print_testcase tests[] = {
	    test<config>(R"(wersja: 1
płatnik:
  nazwisko: 'Nowak, Jan'
  paszport: AB4123456
  nip: 7680002466
  pesel: 26211012346
ubezpieczeni:
  - nazwisko: 'Iksiński, Piotr'
    tytuł ubezpieczenia: 0110 0 0
    pesel: 50671500000
  - nazwisko: 'Iksińska, Maria'
    tytuł ubezpieczenia: 9999 8 7
    pesel: 26211012346
    pensja: 7500 zł
    wymiar: 3/4
)"sv)
	        .on<verbose::names_only>(R"(-- payer: Jan Nowak
-- insured:
--   - Piotr Iksiński
--   - Maria Iksińska
)"sv)
	        .on<verbose::names_and_summary>(
	            R"(-- payer: Jan Nowak (7680002466)
-- insured:
--   - Piotr Iksiński (50671500000), 1/1 of <minimal pay>
--   - Maria Iksińska (26211012346), 3/4 of 7500 zł
)"sv)
	        .on<verbose::names_and_details>(R"(-- payer:
--   name: Jan Nowak
--   social id: 26211012346
--   tax id: 7680002466
--   ident: 2 AB4123456
-- insured:
--   - name: Piotr Iksiński
--     insurance title: 0110 0 0
--     ident: P 50671500000
--     salary: 1/1 of <minimal pay>
--   - name: Maria Iksińska
--     insurance title: 9999 8 7
--     ident: P 26211012346
--     salary: 3/4 of 7500 zł
)"sv)
	        .on<verbose::parameters,
	            verbose::raw_form_data,
	            verbose::templates,
	            verbose::calculated_sections,
	            verbose::last>(R"(-- payer:
--   name: Jan Nowak
--   social id: 26211012346
--   tax id: 7680002466
--   ident: 2 AB4123456
-- insured:
--   - name: Piotr Iksiński
--     insurance title: 0110 0 0
--     ident: P 50671500000
--     salary: 1/1 of <minimal pay>
--   - name: Maria Iksińska
--     insurance title: 9999 8 7
--     ident: P 26211012346
--     salary: 3/4 of 7500 zł
-- parameters
--   cost of obtaining: 250 zł / 300 zł
--   health: insured 9%
--   pension insurance: payer 6.5%, insured 1.5%
--   disability insurance: payer 0%
--   health insurance: payer 9.76%, insured 9.76%
--   accident insurance: payer 1.67%
--   tax scale for month reported:
--     over 30000 zł at 17%
--     over 120000 zł at 32%
)"sv),

	    test<tax_config>(R"(version: 1

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
)"sv)
	        .on<verbose::parameters,
	            verbose::raw_form_data,
	            verbose::templates,
	            verbose::calculated_sections,
	            verbose::last>(R"(-- costs of obtaining per month:
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
)"sv),

	    test<templates>(R"(version: 1
reports:
  DRA:
    - id: I
      fields:
        1: 6
        2: [$serial.NN, $serial.DATE]

    - id: II
      fields:
        1: $payer.tax_id
        3: $payer.social_id
        4: $payer.document_kind
        5: $payer.document
        6: $payer.name
        7: $payer.last
        8: $payer.first
        9: $payer.birthday

    - id: III
      fields:
        1: $insured_count
        3: $accident_insurance_contribution

    - id: IV
      fields:
        1: $+4,7,10,16
        2: $+5,8,11,17
        3: $+1,2
        #
        4: $pension_insurance.insured
        5: $disability_insurance.insured
        6: $+4,5
        7: $pension_insurance.payer
        8: $disability_insurance.payer
        9: $+7,8
        10: 0zł
        11: 0zł
        12: 0zł
        13: 0zł
        14: 0zł
        15: 0zł
        16: 0zł
        17: 0zł
        18: 0zł
        #
        19: $+22,25,28
        20: $+23,26,29,35
        21: $+19,20
        22: $health_insurance.insured
        23: $accident_insurance.insured
        24: $+22,23
        25: $health_insurance.payer
        26: $accident_insurance.payer
        27: $+25,26
        28: 0zł
        29: 0zł
        30: 0zł
        31: 0zł
        32: 0zł
        33: 0zł
        34: 0zł
        35: 0zł
        36: 0zł
        #
        37: $+6,9,24,27

    - id: V
      fields:
        1: 0zł
        2: 0zł
        3: 0zł
        4: 0zł
        5: 0zł

    - id: VI
      fields:
        2: 0zł
        5: 0zł
        6: 0zł
        7: 0zł

    - id: VII
      fields:
        1: 0zł
        2: 0zł
        3: 0zł

    - id: VIII
      fields:
        3: 0zł

    - id: IX
      fields:
        1: 0zł
        2: $insurance_total

    - id: XIII
      fields:
        1: $today

  RCA:
    - id: I
      fields:
        1: [$serial.NN, $serial.DATE]
    - id: II
      fields:
        1: $payer.tax_id
        3: $payer.social_id
        4: $payer.document_kind
        5: $payer.document
        6: $payer.name
        7: $payer.last
        8: $payer.first
        9: $payer.birthday

    - id: III
      repeatable: true

    - id: III
      block: A
      fields:
        1: $insured.last
        2: $insured.first
        3: $insured.document_kind
        4: $insured.document

    - id: III
      block: B
      fields:
        1: $insurance_title
        3: [$scale.num, $scale.den]
        #
        4: $salary.gross
        5: $salary.gross
        6: $salary.gross
        #
        7: $pension_insurance.insured
        8: $disability_insurance.insured
        9: $health_insurance.insured
        10: $accident_insurance.insured
        #
        11: $pension_insurance.payer
        12: $disability_insurance.payer
        13: $health_insurance.payer
        14: $accident_insurance.payer
        #
        29: $+7,8,9,10,11,12,13,14

    - id: III
      block: C
      fields:
        1: $health_baseline
        4: $health_contribution

    - id: III
      block: D
      fields:
        1: 0zł
        2: 0zł
        3: 0zł
        4: 0zł

    - id: IV
      fields:
        1: $today
)"sv)
	        .on<verbose::templates>(R"(-- templates:
--   [DRA]:
--     I:
--       fields:
--         1: '6'
--         2: [$serial.NN, $serial.DATE]
--
--     II:
--       fields:
--         1: $payer.tax_id
--         3: $payer.social_id
--         4: $payer.document_kind
--         5: $payer.document
--         6: $payer.name
--         7: $payer.last
--         8: $payer.first
--         9: $payer.birthday
--
--     III:
--       fields:
--         1: $insured_count
--         3: $accident_insurance_contribution
--
--     IV:
--       fields:
--          1: (4 + 7 + 10 + 16)
--          2: (5 + 8 + 11 + 17)
--          3: (1 + 2)
--          4: $pension_insurance.insured
--          5: $disability_insurance.insured
--          6: (4 + 5)
--          7: $pension_insurance.payer
--          8: $disability_insurance.payer
--          9: (7 + 8)
--         10: 0.00 zł
--         11: 0.00 zł
--         12: 0.00 zł
--         13: 0.00 zł
--         14: 0.00 zł
--         15: 0.00 zł
--         16: 0.00 zł
--         17: 0.00 zł
--         18: 0.00 zł
--         19: (22 + 25 + 28)
--         20: (23 + 26 + 29 + 35)
--         21: (19 + 20)
--         22: $health_insurance.insured
--         23: $accident_insurance.insured
--         24: (22 + 23)
--         25: $health_insurance.payer
--         26: $accident_insurance.payer
--         27: (25 + 26)
--         28: 0.00 zł
--         29: 0.00 zł
--         30: 0.00 zł
--         31: 0.00 zł
--         32: 0.00 zł
--         33: 0.00 zł
--         34: 0.00 zł
--         35: 0.00 zł
--         36: 0.00 zł
--         37: (6 + 9 + 24 + 27)
--
--     V:
--       fields:
--         1: 0.00 zł
--         2: 0.00 zł
--         3: 0.00 zł
--         4: 0.00 zł
--         5: 0.00 zł
--
--     VI:
--       fields:
--         2: 0.00 zł
--         5: 0.00 zł
--         6: 0.00 zł
--         7: 0.00 zł
--
--     VII:
--       fields:
--         1: 0.00 zł
--         2: 0.00 zł
--         3: 0.00 zł
--
--     VIII:
--       fields:
--         3: 0.00 zł
--
--     IX:
--       fields:
--         1: 0.00 zł
--         2: $insurance_total
--
--     XIII:
--       fields:
--         1: $today
--
--   [RCA]:
--     I:
--       fields:
--         1: [$serial.NN, $serial.DATE]
--
--     II:
--       fields:
--         1: $payer.tax_id
--         3: $payer.social_id
--         4: $payer.document_kind
--         5: $payer.document
--         6: $payer.name
--         7: $payer.last
--         8: $payer.first
--         9: $payer.birthday
--
--     III[*]:
--       - id: A
--         fields:
--           1: $insured.last
--           2: $insured.first
--           3: $insured.document_kind
--           4: $insured.document
--
--       - id: B
--         fields:
--            1: $insurance_title
--            3: [$scale.num, $scale.den]
--            4: $salary.gross
--            5: $salary.gross
--            6: $salary.gross
--            7: $pension_insurance.insured
--            8: $disability_insurance.insured
--            9: $health_insurance.insured
--           10: $accident_insurance.insured
--           11: $pension_insurance.payer
--           12: $disability_insurance.payer
--           13: $health_insurance.payer
--           14: $accident_insurance.payer
--           29: (7 + 8 + 9 + 10 + 11 + 12 + 13 + 14)
--
--       - id: C
--         fields:
--           1: $health_baseline
--           4: $health_contribution
--
--       - id: D
--         fields:
--           1: 0.00 zł
--           2: 0.00 zł
--           3: 0.00 zł
--           4: 0.00 zł
--
--     IV:
--       fields:
--         1: $today
--
)"sv),
	};

	INSTANTIATE_TEST_SUITE_P(test, parser_debug, ::testing::ValuesIn(tests));
}  // namespace quick_dra::testing
