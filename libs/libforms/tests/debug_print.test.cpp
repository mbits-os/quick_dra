// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <array>
#include <quick_dra/docs/file_set.hpp>
#include <quick_dra/docs/forms.hpp>
#include <quick_dra/models/project_reader.hpp>
#include <sstream>

namespace quick_dra::testing {
	using std::literals::operator""y;

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

	struct testcase {
		std::string_view config_yaml, templates_yaml;
		verbose level{};
		std::string_view expected{};

		[[nodiscard]] constexpr testcase on(verbose lvl, std::string_view output) const noexcept {
			auto copy = *this;
			copy.level = lvl;
			copy.expected = output;
			return copy;
		}

		[[nodiscard]] std::string capture() const {
			::testing::internal::CaptureStdout();
			run_captured();
			return ::testing::internal::GetCapturedStdout();
		}

		void run_captured() const;
	};

	class debug_print : public ::testing::TestWithParam<testcase> {
	public:
	};

	TEST_P(debug_print, format) {
		auto const& test = GetParam();
		auto const actual = test.capture();
		auto const expected = test.expected;
		ASSERT_EQ(actual, expected) << "Debug level: " << to_str(test.level);
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

	compiled_templates postproc_load(templates& src) { return compiled_templates::compile(src); }

	template <typename FileObj>
	auto load(std::string_view text) {
		auto object = parser::parse_yaml_text<FileObj>({text.data(), text.size()}, "input"s);
		return object.transform([](auto& value) { return postproc_load(value); });
	}

	void testcase::run_captured() const {
		auto const cfg = load<config>(config_yaml);
		auto const compiled = load<templates>(templates_yaml);

		if (!cfg || !compiled) return;

		auto const forms = prepare_form_set(level, 1, 2016y / 1, 2016y / 2 / 10, *cfg);

		build_file_set({.verbose_level = level}, forms, *compiled);
	}

	static constexpr auto test_config = testcase{
	    .config_yaml{R"(wersja: 1
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
)"sv},
	    .templates_yaml{R"(version: 1
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
        1: $today)"sv},
	};

	static constexpr auto raw_form_data = R"(-- form data:
--   RCA [50671500000]:
--      accident_insurance:
--        insured: 0.00 zł
--        payer: 80.16 zł
--      disability_insurance:
--        insured: 0.00 zł
--        payer: 0.00 zł
--      guaranteed_employee_benefits_fund:
--        insured: 0.00 zł
--        payer: 0.00 zł
--      health_baseline: 4010.00 zł
--      health_contribution: 0.00 zł
--      health_insurance:
--        insured: 468.48 zł
--        payer: 468.48 zł
--      insurance_title: ['0110', '0', '0']
--      insurance_total: 1401.12 zł
--      insured:
--        document: '50671500000'
--        document_kind: 'P'
--        first: 'PIOTR'
--        last: 'IKSIŃSKI'
--      payer:
--        birthday: '2026-01-10'
--        document: 'AB4123456'
--        document_kind: '2'
--        first: 'JAN'
--        last: 'NOWAK'
--        name: 'JAN NOWAK'
--        social_id: '26211012346'
--        tax_id: '7680002466'
--      pension_insurance:
--        insured: 72.00 zł
--        payer: 312.00 zł
--      salary:
--        gross: 4800.00 zł
--        net: 4002.82 zł
--        payer_gross: 5660.64 zł
--      scale:
--        den: 1
--        num: 1
--      serial:
--        DATE: '2016-01'
--        NN: '01'
--      tax_total: 256.70 zł
--      today: '2016-02-10'
--
--   RCA [26211012346]:
--      accident_insurance:
--        insured: 0.00 zł
--        payer: 93.94 zł
--      disability_insurance:
--        insured: 0.00 zł
--        payer: 0.00 zł
--      guaranteed_employee_benefits_fund:
--        insured: 0.00 zł
--        payer: 0.00 zł
--      health_baseline: 4742.00 zł
--      health_contribution: 0.00 zł
--      health_insurance:
--        insured: 549.00 zł
--        payer: 549.00 zł
--      insurance_title: ['9999', '8', '7']
--      insurance_total: 1641.95 zł
--      insured:
--        document: '26211012346'
--        document_kind: 'P'
--        first: 'MARIA'
--        last: 'IKSIŃSKA'
--      payer:
--        birthday: '2026-01-10'
--        document: 'AB4123456'
--        document_kind: '2'
--        first: 'JAN'
--        last: 'NOWAK'
--        name: 'JAN NOWAK'
--        social_id: '26211012346'
--        tax_id: '7680002466'
--      pension_insurance:
--        insured: 84.38 zł
--        payer: 365.63 zł
--      salary:
--        gross: 5625.00 zł
--        net: 4610.48 zł
--        payer_gross: 6633.57 zł
--      scale:
--        den: 4
--        num: 3
--      serial:
--        DATE: '2016-01'
--        NN: '01'
--      tax_total: 381.14 zł
--      today: '2016-02-10'
--
--   DRA:
--      accident_insurance:
--        insured: 0.00 zł
--        payer: 174.10 zł
--      accident_insurance_contribution: 1.67%
--      disability_insurance:
--        insured: 0.00 zł
--        payer: 0.00 zł
--      health_insurance:
--        insured: 1017.48 zł
--        payer: 1017.48 zł
--      insurance_total: 3043.07 zł
--      insured_count: 2
--      payer:
--        birthday: '2026-01-10'
--        document: 'AB4123456'
--        document_kind: '2'
--        first: 'JAN'
--        last: 'NOWAK'
--        name: 'JAN NOWAK'
--        social_id: '26211012346'
--        tax_id: '7680002466'
--      pension_insurance:
--        insured: 156.38 zł
--        payer: 677.63 zł
--      serial:
--        DATE: '2016-01'
--        NN: '01'
--      tax_total: 637.84 zł
--      today: '2016-02-10'
--
)"sv;

	static constexpr auto verbose_levels = std::array{
	    std::pair{verbose::none, R"()"sv},

	    std::pair{verbose::names_only, R"()"sv},

	    std::pair{verbose::names_and_summary, R"()"sv},

	    std::pair{verbose::names_and_details, R"()"sv},

	    std::pair{verbose::parameters, R"()"sv},

	    std::pair{verbose::raw_form_data, raw_form_data},

	    std::pair{verbose::templates, R"(-- form data:
--   RCA [50671500000]:
--      accident_insurance:
--        insured: 0.00 zł
--        payer: 80.16 zł
--      disability_insurance:
--        insured: 0.00 zł
--        payer: 0.00 zł
--      guaranteed_employee_benefits_fund:
--        insured: 0.00 zł
--        payer: 0.00 zł
--      health_baseline: 4010.00 zł
--      health_contribution: 0.00 zł
--      health_insurance:
--        insured: 468.48 zł
--        payer: 468.48 zł
--      insurance_title: ['0110', '0', '0']
--      insurance_total: 1401.12 zł
--      insured:
--        document: '50671500000'
--        document_kind: 'P'
--        first: 'PIOTR'
--        last: 'IKSIŃSKI'
--      payer:
--        birthday: '2026-01-10'
--        document: 'AB4123456'
--        document_kind: '2'
--        first: 'JAN'
--        last: 'NOWAK'
--        name: 'JAN NOWAK'
--        social_id: '26211012346'
--        tax_id: '7680002466'
--      pension_insurance:
--        insured: 72.00 zł
--        payer: 312.00 zł
--      salary:
--        gross: 4800.00 zł
--        net: 4002.82 zł
--        payer_gross: 5660.64 zł
--      scale:
--        den: 1
--        num: 1
--      serial:
--        DATE: '2016-01'
--        NN: '01'
--      tax_total: 256.70 zł
--      today: '2016-02-10'
--
--   RCA [26211012346]:
--      accident_insurance:
--        insured: 0.00 zł
--        payer: 93.94 zł
--      disability_insurance:
--        insured: 0.00 zł
--        payer: 0.00 zł
--      guaranteed_employee_benefits_fund:
--        insured: 0.00 zł
--        payer: 0.00 zł
--      health_baseline: 4742.00 zł
--      health_contribution: 0.00 zł
--      health_insurance:
--        insured: 549.00 zł
--        payer: 549.00 zł
--      insurance_title: ['9999', '8', '7']
--      insurance_total: 1641.95 zł
--      insured:
--        document: '26211012346'
--        document_kind: 'P'
--        first: 'MARIA'
--        last: 'IKSIŃSKA'
--      payer:
--        birthday: '2026-01-10'
--        document: 'AB4123456'
--        document_kind: '2'
--        first: 'JAN'
--        last: 'NOWAK'
--        name: 'JAN NOWAK'
--        social_id: '26211012346'
--        tax_id: '7680002466'
--      pension_insurance:
--        insured: 84.38 zł
--        payer: 365.63 zł
--      salary:
--        gross: 5625.00 zł
--        net: 4610.48 zł
--        payer_gross: 6633.57 zł
--      scale:
--        den: 4
--        num: 3
--      serial:
--        DATE: '2016-01'
--        NN: '01'
--      tax_total: 381.14 zł
--      today: '2016-02-10'
--
--   DRA:
--      accident_insurance:
--        insured: 0.00 zł
--        payer: 174.10 zł
--      accident_insurance_contribution: 1.67%
--      disability_insurance:
--        insured: 0.00 zł
--        payer: 0.00 zł
--      health_insurance:
--        insured: 1017.48 zł
--        payer: 1017.48 zł
--      insurance_total: 3043.07 zł
--      insured_count: 2
--      payer:
--        birthday: '2026-01-10'
--        document: 'AB4123456'
--        document_kind: '2'
--        first: 'JAN'
--        last: 'NOWAK'
--        name: 'JAN NOWAK'
--        social_id: '26211012346'
--        tax_id: '7680002466'
--      pension_insurance:
--        insured: 156.38 zł
--        payer: 677.63 zł
--      serial:
--        DATE: '2016-01'
--        NN: '01'
--      tax_total: 637.84 zł
--      today: '2016-02-10'
--
-- templates:
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
)"sv},

	    std::pair{verbose::calculated_sections, R"(-- form data:
--   RCA [50671500000]:
--      accident_insurance:
--        insured: 0.00 zł
--        payer: 80.16 zł
--      disability_insurance:
--        insured: 0.00 zł
--        payer: 0.00 zł
--      guaranteed_employee_benefits_fund:
--        insured: 0.00 zł
--        payer: 0.00 zł
--      health_baseline: 4010.00 zł
--      health_contribution: 0.00 zł
--      health_insurance:
--        insured: 468.48 zł
--        payer: 468.48 zł
--      insurance_title: ['0110', '0', '0']
--      insurance_total: 1401.12 zł
--      insured:
--        document: '50671500000'
--        document_kind: 'P'
--        first: 'PIOTR'
--        last: 'IKSIŃSKI'
--      payer:
--        birthday: '2026-01-10'
--        document: 'AB4123456'
--        document_kind: '2'
--        first: 'JAN'
--        last: 'NOWAK'
--        name: 'JAN NOWAK'
--        social_id: '26211012346'
--        tax_id: '7680002466'
--      pension_insurance:
--        insured: 72.00 zł
--        payer: 312.00 zł
--      salary:
--        gross: 4800.00 zł
--        net: 4002.82 zł
--        payer_gross: 5660.64 zł
--      scale:
--        den: 1
--        num: 1
--      serial:
--        DATE: '2016-01'
--        NN: '01'
--      tax_total: 256.70 zł
--      today: '2016-02-10'
--
--   RCA [26211012346]:
--      accident_insurance:
--        insured: 0.00 zł
--        payer: 93.94 zł
--      disability_insurance:
--        insured: 0.00 zł
--        payer: 0.00 zł
--      guaranteed_employee_benefits_fund:
--        insured: 0.00 zł
--        payer: 0.00 zł
--      health_baseline: 4742.00 zł
--      health_contribution: 0.00 zł
--      health_insurance:
--        insured: 549.00 zł
--        payer: 549.00 zł
--      insurance_title: ['9999', '8', '7']
--      insurance_total: 1641.95 zł
--      insured:
--        document: '26211012346'
--        document_kind: 'P'
--        first: 'MARIA'
--        last: 'IKSIŃSKA'
--      payer:
--        birthday: '2026-01-10'
--        document: 'AB4123456'
--        document_kind: '2'
--        first: 'JAN'
--        last: 'NOWAK'
--        name: 'JAN NOWAK'
--        social_id: '26211012346'
--        tax_id: '7680002466'
--      pension_insurance:
--        insured: 84.38 zł
--        payer: 365.63 zł
--      salary:
--        gross: 5625.00 zł
--        net: 4610.48 zł
--        payer_gross: 6633.57 zł
--      scale:
--        den: 4
--        num: 3
--      serial:
--        DATE: '2016-01'
--        NN: '01'
--      tax_total: 381.14 zł
--      today: '2016-02-10'
--
--   DRA:
--      accident_insurance:
--        insured: 0.00 zł
--        payer: 174.10 zł
--      accident_insurance_contribution: 1.67%
--      disability_insurance:
--        insured: 0.00 zł
--        payer: 0.00 zł
--      health_insurance:
--        insured: 1017.48 zł
--        payer: 1017.48 zł
--      insurance_total: 3043.07 zł
--      insured_count: 2
--      payer:
--        birthday: '2026-01-10'
--        document: 'AB4123456'
--        document_kind: '2'
--        first: 'JAN'
--        last: 'NOWAK'
--        name: 'JAN NOWAK'
--        social_id: '26211012346'
--        tax_id: '7680002466'
--      pension_insurance:
--        insured: 156.38 zł
--        payer: 677.63 zł
--      serial:
--        DATE: '2016-01'
--        NN: '01'
--      tax_total: 637.84 zł
--      today: '2016-02-10'
--
-- filled forms:
--   ZUSRCA [50671500000]
--     I:
--       fields:
--         1: ['01', '2016-01']
--
--     II:
--       fields:
--         1: '7680002466'
--         3: '26211012346'
--         4: '2'
--         5: 'AB4123456'
--         6: 'JAN NOWAK'
--         7: 'NOWAK'
--         8: 'JAN'
--         9: '2026-01-10'
--
--     III[*]:
--       - id: A
--         fields:
--           1: 'IKSIŃSKI'
--           2: 'PIOTR'
--           3: 'P'
--           4: '50671500000'
--
--       - id: B
--         fields:
--            1: ['0110', '0', '0']
--            3: [1, 1]
--            4: 4800.00 zł
--            5: 4800.00 zł
--            6: 4800.00 zł
--            7: 72.00 zł
--            8: 0.00 zł
--            9: 468.48 zł
--           10: 0.00 zł
--           11: 312.00 zł
--           12: 0.00 zł
--           13: 468.48 zł
--           14: 80.16 zł
--           29: 1401.12 zł
--
--       - id: C
--         fields:
--           1: 4010.00 zł
--           4: 0.00 zł
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
--         1: '2016-02-10'
--
--   ZUSRCA [26211012346]
--     I:
--       fields:
--         1: ['01', '2016-01']
--
--     II:
--       fields:
--         1: '7680002466'
--         3: '26211012346'
--         4: '2'
--         5: 'AB4123456'
--         6: 'JAN NOWAK'
--         7: 'NOWAK'
--         8: 'JAN'
--         9: '2026-01-10'
--
--     III[*]:
--       - id: A
--         fields:
--           1: 'IKSIŃSKA'
--           2: 'MARIA'
--           3: 'P'
--           4: '26211012346'
--
--       - id: B
--         fields:
--            1: ['9999', '8', '7']
--            3: [3, 4]
--            4: 5625.00 zł
--            5: 5625.00 zł
--            6: 5625.00 zł
--            7: 84.38 zł
--            8: 0.00 zł
--            9: 549.00 zł
--           10: 0.00 zł
--           11: 365.63 zł
--           12: 0.00 zł
--           13: 549.00 zł
--           14: 93.94 zł
--           29: 1641.95 zł
--
--       - id: C
--         fields:
--           1: 4742.00 zł
--           4: 0.00 zł
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
--         1: '2016-02-10'
--
--   ZUSDRA
--     I:
--       fields:
--         1: '6'
--         2: ['01', '2016-01']
--
--     II:
--       fields:
--         1: '7680002466'
--         3: '26211012346'
--         4: '2'
--         5: 'AB4123456'
--         6: 'JAN NOWAK'
--         7: 'NOWAK'
--         8: 'JAN'
--         9: '2026-01-10'
--
--     III:
--       fields:
--         1: 2
--         3: 1.67%
--
--     IV:
--       fields:
--          1: 834.01 zł
--          2: 0.00 zł
--          3: 834.01 zł
--          4: 156.38 zł
--          5: 0.00 zł
--          6: 156.38 zł
--          7: 677.63 zł
--          8: 0.00 zł
--          9: 677.63 zł
--         10: 0.00 zł
--         11: 0.00 zł
--         12: 0.00 zł
--         13: 0.00 zł
--         14: 0.00 zł
--         15: 0.00 zł
--         16: 0.00 zł
--         17: 0.00 zł
--         18: 0.00 zł
--         19: 2034.96 zł
--         20: 174.10 zł
--         21: 2209.06 zł
--         22: 1017.48 zł
--         23: 0.00 zł
--         24: 1017.48 zł
--         25: 1017.48 zł
--         26: 174.10 zł
--         27: 1191.58 zł
--         28: 0.00 zł
--         29: 0.00 zł
--         30: 0.00 zł
--         31: 0.00 zł
--         32: 0.00 zł
--         33: 0.00 zł
--         34: 0.00 zł
--         35: 0.00 zł
--         36: 0.00 zł
--         37: 3043.07 zł
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
--         2: 3043.07 zł
--
--     XIII:
--       fields:
--         1: '2016-02-10'
--
)"sv},

	    std::pair{verbose::last, raw_form_data},
	};

	testcase transformer(std::pair<verbose, std::string_view> const& pair) {
		return test_config.on(pair.first, pair.second);
	}

	inline std::vector<testcase> levels_to_vector() {
		std::vector<testcase> result{};
		for (auto const& [level, output] : verbose_levels) {
			result.emplace_back(test_config.on(level, output));
		}
		return result;
	}

	auto const tests = levels_to_vector();

	INSTANTIATE_TEST_SUITE_P(level, debug_print, ::testing::ValuesIn(tests));

}  // namespace quick_dra::testing
