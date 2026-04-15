// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "run.hpp"

namespace quick_dra::builtin::testing::xml {
	namespace {
		using namespace std::chrono;

		template <typename D1, typename D2, typename Clock>
		time_point<Clock, D1> floor(time_point<Clock, D1> const& from) {
			auto const orig_dur = from.time_since_epoch();
			auto const casted_dur = duration_cast<D1>(orig_dur);
			return time_point<Clock, D1>{casted_dur};
		}

		year_month_day get_today() {
			auto const now = system_clock::now();
			auto const local = floor<days>(zoned_time{current_zone(), now}.get_local_time());
			return year_month_day{local};
		}

		std::string no_config_stdout() {
			auto const today = get_today();
			auto const date = year_month{today.year(), today.month()} - months{1};
			return fmt::format(R"(-- report: #1 {:04}-{:02}
Quick-DRA: error: cannot find .quick_dra.yaml
)",
			                   static_cast<int>(date.year()), static_cast<unsigned>(date.month()));
			;
		}
	}  // namespace
	static constexpr runnable_testcase tests[] = {
	    {
	        .name = "minimal_pay not pretty"sv,
	        .args = "xml --info --today 2026-1-1 --config .quick_dra.yaml"sv,
	        .config = R"(wersja: 1
płatnik:
  nazwisko: 'Nowak, Jan'
  paszport: AB4123456
  nip: 7680002466
  pesel: 26211012346
ubezpieczeni:
  - nazwisko: 'Iksiński, Piotr'
    tytuł ubezpieczenia: 0110 0 0
    pesel: 50671500000
)"sv,
	        .stdout = R"(-- report: #1 2025-12
-- output: quick-dra_202512-01.xml
-- payments:
   - PIOTR IKSIŃSKI: 3873.17 zł
   - ZUS:            1476.32 zł
   - Urząd Skarbowy:  153.12 zł
   sum total =       5502.61 zł
)"sv,
	        .writes =
	            new_file{
	                .name = "quick-dra_202512-01.xml"sv,
	                .cmp = "quick-dra_202512-01.AB4123456_50671500000_not-pretty.xml"sv,
	            },
	    },
	    {
	        .name = "minimal_pay"sv,
	        .args = "xml --pretty --info --today 2026-1-1 --config .quick_dra.yaml"sv,
	        .config = R"(wersja: 1
płatnik:
  nazwisko: 'Nowak, Jan'
  paszport: AB4123456
  nip: 7680002466
  pesel: 26211012346
ubezpieczeni:
  - nazwisko: 'Iksiński, Piotr'
    tytuł ubezpieczenia: 0110 0 0
    pesel: 50671500000
)"sv,
	        .stdout = R"(-- report: #1 2025-12
-- output: quick-dra_202512-01.xml
-- payments:
   - PIOTR IKSIŃSKI: 3873.17 zł
   - ZUS:            1476.32 zł
   - Urząd Skarbowy:  153.12 zł
   sum total =       5502.61 zł
)"sv,
	        .writes =
	            new_file{
	                .name = "quick-dra_202512-01.xml"sv,
	                .cmp = "quick-dra_202512-01.AB4123456_50671500000.xml"sv,
	            },
	    },
	    {
	        .name = "one quarter"sv,
	        .args = "xml --pretty --info --today 2026-2-1 --config .quick_dra.yaml -vvv"sv,
	        .config = R"(wersja: 1
płatnik:
  nazwisko: 'Nowak, Jan'
  paszport: AB4123456
  nip: 7680002466
  pesel: 26211012346
ubezpieczeni:
  - nazwisko: 'Iksiński, Piotr'
    tytuł ubezpieczenia: 0110 0 0
    pesel: 50671500000
    wymiar: 1/4
)"sv,
	        .stdout = R"(-- config used: .quick_dra.yaml
-- today: 2026-02-01
-- report: #1 2026-01
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
--   minimal pay for month reported: 4806.00 zł
-- output: quick-dra_202601-01.xml
-- payments:
   - PIOTR IKSIŃSKI: 1036.77 zł
   - ZUS:             380.17 zł
   - Urząd Skarbowy:    0.00 zł
   sum total =       1416.94 zł
)"sv,
	        .writes =
	            new_file{
	                .name = "quick-dra_202601-01.xml"sv,
	                .cmp = "quick-dra_202601-01.AB4123456_50671500000.quarter.xml"sv,
	            },
	    },
	    {
	        // has "check"
	        .name = "verbosity 6"sv,
	        .args = "xml --pretty --today 2026-2-1 -vvvvvv --config .quick_dra.yaml"sv,
	        .config = R"(wersja: 1
płatnik:
  nazwisko: "Nowak (HOME), Jan"
  paszport: AB4123456
  nip: 7680002466
  pesel: 26211012346
ubezpieczeni:
  - nazwisko: Ubezpieczona, Osoba
    tytuł ubezpieczenia: 0110 1 1
    pesel: 50671500000
    wymiar: 1/4
    pensja: 8000
)"sv,
	        .stdout = R"(-- payer:
--   name: Jan Nowak (HOME)
--   social id: 26211012346
--   tax id: 7680002466
--   ident: 2 AB4123456
-- insured:
--   - name: Osoba Ubezpieczona
--     insurance title: 0110 1 1
--     ident: P 50671500000
--     salary: 1/4 of 8000 zł
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
-- form data:
--   RCA [50671500000]:
--      accident_insurance:
--        insured: 0.00 zł
--        payer: 33.40 zł
--      disability_insurance:
--        insured: 30.00 zł
--        payer: 130.00 zł
--      guaranteed_employee_benefits_fund:
--        insured: 0.00 zł
--        payer: 0.00 zł
--      health_baseline: 1476.00 zł
--      health_contribution: 0.00 zł
--      health_insurance:
--        insured: 49.00 zł
--        payer: 0.00 zł
--      insurance_title: ['0110', '1', '1']
--      insurance_total: 632.80 zł
--      insured:
--        document: '50671500000'
--        document_kind: 'P'
--        first: 'OSOBA'
--        last: 'UBEZPIECZONA'
--      payer:
--        birthday: '2026-01-10'
--        document: 'AB4123456'
--        document_kind: '2'
--        first: 'JAN'
--        last: 'NOWAK (HOME)'
--        name: 'JAN NOWAK (HOME)'
--        social_id: '26211012346'
--        tax_id: '7680002466'
--      pension_insurance:
--        insured: 195.20 zł
--        payer: 195.20 zł
--      salary:
--        gross: 2000.00 zł
--        net: 1725.80 zł
--        payer_gross: 2358.60 zł
--      scale:
--        den: 4
--        num: 1
--      serial:
--        DATE: '2026-01'
--        NN: '01'
--      tax_total: 0.00 zł
--      today: '2026-02-01'
--
--   DRA:
--      accident_insurance:
--        insured: 0.00 zł
--        payer: 33.40 zł
--      accident_insurance_contribution: 1.67%
--      disability_insurance:
--        insured: 30.00 zł
--        payer: 130.00 zł
--      health_insurance:
--        insured: 49.00 zł
--        payer: 0.00 zł
--      insurance_total: 632.80 zł
--      insured_count: 1
--      payer:
--        birthday: '2026-01-10'
--        document: 'AB4123456'
--        document_kind: '2'
--        first: 'JAN'
--        last: 'NOWAK (HOME)'
--        name: 'JAN NOWAK (HOME)'
--        social_id: '26211012346'
--        tax_id: '7680002466'
--      pension_insurance:
--        insured: 195.20 zł
--        payer: 195.20 zł
--      serial:
--        DATE: '2026-01'
--        NN: '01'
--      tax_total: 0.00 zł
--      today: '2026-02-01'
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
-- output: quick-dra_202601-01.xml
-- use --info to print summary of amounts to pay
)"sv,
	        .writes =
	            new_file{
	                .name = "quick-dra_202601-01.xml"sv,
	                .cmp = "homes/var_home/quick-dra_202601-01.xml"sv,
	            },
	        .check_stdout = compare::end,
	    },
	    {
	        // has "check"
	        .name = "verbosity 7"sv,
	        .args = "xml --pretty --today 2026-2-1 -vvvvvvv --config .quick_dra.yaml"sv,
	        .config = R"(wersja: 1
płatnik:
  nazwisko: "Nowak (HOME), Jan"
  paszport: AB4123456
  nip: 7680002466
  pesel: 26211012346
ubezpieczeni:
  - nazwisko: Ubezpieczona, Osoba
    tytuł ubezpieczenia: 0110 1 1
    pesel: 50671500000
    wymiar: 1/4
    pensja: 8000
)"sv,
	        .stdout = R"(-- payer:
--   name: Jan Nowak (HOME)
--   social id: 26211012346
--   tax id: 7680002466
--   ident: 2 AB4123456
-- insured:
--   - name: Osoba Ubezpieczona
--     insurance title: 0110 1 1
--     ident: P 50671500000
--     salary: 1/4 of 8000 zł
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
-- form data:
--   RCA [50671500000]:
--      accident_insurance:
--        insured: 0.00 zł
--        payer: 33.40 zł
--      disability_insurance:
--        insured: 30.00 zł
--        payer: 130.00 zł
--      guaranteed_employee_benefits_fund:
--        insured: 0.00 zł
--        payer: 0.00 zł
--      health_baseline: 1476.00 zł
--      health_contribution: 0.00 zł
--      health_insurance:
--        insured: 49.00 zł
--        payer: 0.00 zł
--      insurance_title: ['0110', '1', '1']
--      insurance_total: 632.80 zł
--      insured:
--        document: '50671500000'
--        document_kind: 'P'
--        first: 'OSOBA'
--        last: 'UBEZPIECZONA'
--      payer:
--        birthday: '2026-01-10'
--        document: 'AB4123456'
--        document_kind: '2'
--        first: 'JAN'
--        last: 'NOWAK (HOME)'
--        name: 'JAN NOWAK (HOME)'
--        social_id: '26211012346'
--        tax_id: '7680002466'
--      pension_insurance:
--        insured: 195.20 zł
--        payer: 195.20 zł
--      salary:
--        gross: 2000.00 zł
--        net: 1725.80 zł
--        payer_gross: 2358.60 zł
--      scale:
--        den: 4
--        num: 1
--      serial:
--        DATE: '2026-01'
--        NN: '01'
--      tax_total: 0.00 zł
--      today: '2026-02-01'
--
--   DRA:
--      accident_insurance:
--        insured: 0.00 zł
--        payer: 33.40 zł
--      accident_insurance_contribution: 1.67%
--      disability_insurance:
--        insured: 30.00 zł
--        payer: 130.00 zł
--      health_insurance:
--        insured: 49.00 zł
--        payer: 0.00 zł
--      insurance_total: 632.80 zł
--      insured_count: 1
--      payer:
--        birthday: '2026-01-10'
--        document: 'AB4123456'
--        document_kind: '2'
--        first: 'JAN'
--        last: 'NOWAK (HOME)'
--        name: 'JAN NOWAK (HOME)'
--        social_id: '26211012346'
--        tax_id: '7680002466'
--      pension_insurance:
--        insured: 195.20 zł
--        payer: 195.20 zł
--      serial:
--        DATE: '2026-01'
--        NN: '01'
--      tax_total: 0.00 zł
--      today: '2026-02-01'
--
-- filled forms:
--   ZUSRCA [50671500000]
--     I:
--       fields:
--         1: ['01', '2026-01']
--
--     II:
--       fields:
--         1: '7680002466'
--         3: '26211012346'
--         4: '2'
--         5: 'AB4123456'
--         6: 'JAN NOWAK (HOME)'
--         7: 'NOWAK (HOME)'
--         8: 'JAN'
--         9: '2026-01-10'
--
--     III[*]:
--       - id: A
--         fields:
--           1: 'UBEZPIECZONA'
--           2: 'OSOBA'
--           3: 'P'
--           4: '50671500000'
--
--       - id: B
--         fields:
--            1: ['0110', '1', '1']
--            3: [1, 4]
--            4: 2000.00 zł
--            5: 2000.00 zł
--            6: 2000.00 zł
--            7: 195.20 zł
--            8: 30.00 zł
--            9: 49.00 zł
--           10: 0.00 zł
--           11: 195.20 zł
--           12: 130.00 zł
--           13: 0.00 zł
--           14: 33.40 zł
--           29: 632.80 zł
--
--       - id: C
--         fields:
--           1: 1476.00 zł
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
--         1: '2026-02-01'
--
--   ZUSDRA
--     I:
--       fields:
--         1: '6'
--         2: ['01', '2026-01']
--
--     II:
--       fields:
--         1: '7680002466'
--         3: '26211012346'
--         4: '2'
--         5: 'AB4123456'
--         6: 'JAN NOWAK (HOME)'
--         7: 'NOWAK (HOME)'
--         8: 'JAN'
--         9: '2026-01-10'
--
--     III:
--       fields:
--         1: 1
--         3: 1.67%
--
--     IV:
--       fields:
--          1: 390.40 zł
--          2: 160.00 zł
--          3: 550.40 zł
--          4: 195.20 zł
--          5: 30.00 zł
--          6: 225.20 zł
--          7: 195.20 zł
--          8: 130.00 zł
--          9: 325.20 zł
--         10: 0.00 zł
--         11: 0.00 zł
--         12: 0.00 zł
--         13: 0.00 zł
--         14: 0.00 zł
--         15: 0.00 zł
--         16: 0.00 zł
--         17: 0.00 zł
--         18: 0.00 zł
--         19: 49.00 zł
--         20: 33.40 zł
--         21: 82.40 zł
--         22: 49.00 zł
--         23: 0.00 zł
--         24: 49.00 zł
--         25: 0.00 zł
--         26: 33.40 zł
--         27: 33.40 zł
--         28: 0.00 zł
--         29: 0.00 zł
--         30: 0.00 zł
--         31: 0.00 zł
--         32: 0.00 zł
--         33: 0.00 zł
--         34: 0.00 zł
--         35: 0.00 zł
--         36: 0.00 zł
--         37: 632.80 zł
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
--         2: 632.80 zł
--
--     XIII:
--       fields:
--         1: '2026-02-01'
--
-- output: quick-dra_202601-01.xml
-- use --info to print summary of amounts to pay
)"sv,
	        .writes =
	            new_file{
	                .name = "quick-dra_202601-01.xml"sv,
	                .cmp = "homes/var_home/quick-dra_202601-01.xml"sv,
	            },
	        .check_stdout = compare::end,
	    },
	    {
	        // has "check"
	        .name = "verbosity 8"sv,
	        .args = "xml --pretty --today 2026-2-1 -vvvvvvvv --config .quick_dra.yaml"sv,
	        .config = R"(wersja: 1
płatnik:
  nazwisko: "Nowak (HOME), Jan"
  paszport: AB4123456
  nip: 7680002466
  pesel: 26211012346
ubezpieczeni:
  - nazwisko: Ubezpieczona, Osoba
    tytuł ubezpieczenia: 0110 1 1
    pesel: 50671500000
    wymiar: 1/4
    pensja: 8000
)"sv,
	        .stdout = R"(-- payer:
--   name: Jan Nowak (HOME)
--   social id: 26211012346
--   tax id: 7680002466
--   ident: 2 AB4123456
-- insured:
--   - name: Osoba Ubezpieczona
--     insurance title: 0110 1 1
--     ident: P 50671500000
--     salary: 1/4 of 8000 zł
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
-- form data:
--   RCA [50671500000]:
--      accident_insurance:
--        insured: 0.00 zł
--        payer: 33.40 zł
--      disability_insurance:
--        insured: 30.00 zł
--        payer: 130.00 zł
--      guaranteed_employee_benefits_fund:
--        insured: 0.00 zł
--        payer: 0.00 zł
--      health_baseline: 1476.00 zł
--      health_contribution: 0.00 zł
--      health_insurance:
--        insured: 49.00 zł
--        payer: 0.00 zł
--      insurance_title: ['0110', '1', '1']
--      insurance_total: 632.80 zł
--      insured:
--        document: '50671500000'
--        document_kind: 'P'
--        first: 'OSOBA'
--        last: 'UBEZPIECZONA'
--      payer:
--        birthday: '2026-01-10'
--        document: 'AB4123456'
--        document_kind: '2'
--        first: 'JAN'
--        last: 'NOWAK (HOME)'
--        name: 'JAN NOWAK (HOME)'
--        social_id: '26211012346'
--        tax_id: '7680002466'
--      pension_insurance:
--        insured: 195.20 zł
--        payer: 195.20 zł
--      salary:
--        gross: 2000.00 zł
--        net: 1725.80 zł
--        payer_gross: 2358.60 zł
--      scale:
--        den: 4
--        num: 1
--      serial:
--        DATE: '2026-01'
--        NN: '01'
--      tax_total: 0.00 zł
--      today: '2026-02-01'
--
--   DRA:
--      accident_insurance:
--        insured: 0.00 zł
--        payer: 33.40 zł
--      accident_insurance_contribution: 1.67%
--      disability_insurance:
--        insured: 30.00 zł
--        payer: 130.00 zł
--      health_insurance:
--        insured: 49.00 zł
--        payer: 0.00 zł
--      insurance_total: 632.80 zł
--      insured_count: 1
--      payer:
--        birthday: '2026-01-10'
--        document: 'AB4123456'
--        document_kind: '2'
--        first: 'JAN'
--        last: 'NOWAK (HOME)'
--        name: 'JAN NOWAK (HOME)'
--        social_id: '26211012346'
--        tax_id: '7680002466'
--      pension_insurance:
--        insured: 195.20 zł
--        payer: 195.20 zł
--      serial:
--        DATE: '2026-01'
--        NN: '01'
--      tax_total: 0.00 zł
--      today: '2026-02-01'
--
-- output: quick-dra_202601-01.xml
-- use --info to print summary of amounts to pay
-- (no more info to unveil)
)"sv,
	        .writes =
	            new_file{
	                .name = "quick-dra_202601-01.xml"sv,
	                .cmp = "homes/var_home/quick-dra_202601-01.xml"sv,
	            },
	        .check_stdout = compare::end,
	    },
	    {
	        .name = "no config"sv,
	        .args = "xml --config .quick_dra.yaml"sv,
	        .stdout = no_config_stdout,
	        .returncode = 1,
	    },
	    {
	        .name = "today not matching A"sv,
	        .args = "xml --pretty --info --today 2026-14-34 --config .quick_dra.yaml"sv,
	        .config = R"(wersja: 1
płatnik:
  nazwisko: 'Nowak, Jan'
  paszport: AB4123456
  nip: 7680002466
  pesel: 26211012346
ubezpieczeni:
  - nazwisko: 'Iksiński, Piotr'
    tytuł ubezpieczenia: 0110 0 0
    pesel: 50671500000
)"sv,
	        .stderr =
	            R"(usage: qdra xml [-h] [-v ...] [--config <path>] [--tax-config <path>] [-n <NN>] [-m <month>] [--today <YYYY-MM-DD>] [--pretty] [--info]
qdra xml: error: --today: expected YYYY-MM-DD, got `2026-14-34'
)"sv,
	        .returncode = 2,
	    },
	    {
	        .name = "today not matching B"sv,
	        .args = "xml --pretty --info --today something --config .quick_dra.yaml"sv,
	        .config = R"(wersja: 1
płatnik:
  nazwisko: 'Nowak, Jan'
  paszport: AB4123456
  nip: 7680002466
  pesel: 26211012346
ubezpieczeni:
  - nazwisko: 'Iksiński, Piotr'
    tytuł ubezpieczenia: 0110 0 0
    pesel: 50671500000
)"sv,
	        .stderr =
	            R"(usage: qdra xml [-h] [-v ...] [--config <path>] [--tax-config <path>] [-n <NN>] [-m <month>] [--today <YYYY-MM-DD>] [--pretty] [--info]
qdra xml: error: --today: expected YYYY-MM-DD, got `something'
)"sv,
	        .returncode = 2,
	    },
	    {
	        .name = "today invalid"sv,
	        .args = "xml --pretty --info --today 2026-02-31 --config .quick_dra.yaml"sv,
	        .config = R"(wersja: 1
płatnik:
  nazwisko: 'Nowak, Jan'
  paszport: AB4123456
  nip: 7680002466
  pesel: 26211012346
ubezpieczeni:
  - nazwisko: 'Iksiński, Piotr'
    tytuł ubezpieczenia: 0110 0 0
    pesel: 50671500000
)"sv,
	        .stderr =
	            R"(usage: qdra xml [-h] [-v ...] [--config <path>] [--tax-config <path>] [-n <NN>] [-m <month>] [--today <YYYY-MM-DD>] [--pretty] [--info]
qdra xml: error: --today: expected YYYY-MM-DD, got `2026-02-31'
)"sv,
	        .returncode = 2,
	    },
	    {
	        .name = "report index out of bounds"sv,
	        .args = "xml --pretty --info --today 2026-1-1 -n 199 --config .quick_dra.yaml"sv,
	        .config = R"(wersja: 1
płatnik:
  nazwisko: 'Nowak, Jan'
  paszport: AB4123456
  nip: 7680002466
  pesel: 26211012346
ubezpieczeni:
  - nazwisko: 'Iksiński, Piotr'
    tytuł ubezpieczenia: 0110 0 0
    pesel: 50671500000
)"sv,
	        .stderr =
	            R"(usage: qdra xml [-h] [-v ...] [--config <path>] [--tax-config <path>] [-n <NN>] [-m <month>] [--today <YYYY-MM-DD>] [--pretty] [--info]
qdra xml: error: serial number must be in range 1 to 99 inclusive
)"sv,
	        .returncode = 2,
	    },
	};

	INSTANTIATE_TEST_SUITE_P(xml, cli_test, ::testing::ValuesIn(tests));
}  // namespace quick_dra::builtin::testing::xml
