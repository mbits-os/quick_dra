// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <quick_dra/docs/forms.hpp>
#include <quick_dra/docs/summary.hpp>
#include <sstream>

namespace quick_dra::testing {
	TEST(summary, invalid) {
		std::vector<form> forms{
		    {.key = "DRA"s, .state{}},
		    {.key = "RCA"s, .state{}},
		    {.key = "RCA"s, .state{}},
		};
		auto const summary = gather_summary_data(forms);
		::testing::internal::CaptureStdout();
		print_summary(summary);
		auto const actual = ::testing::internal::GetCapturedStdout();
		auto const expected = R"(-- payments:
   - <brak imienia> <brak nazwiska>: (nieznane)
   - <brak imienia> <brak nazwiska>: (nieznane)
   - ZUS:                            (nieznane)
   - Urząd Skarbowy:                 (nieznane)
   sum total =                          0.00 zł
)"sv;
		ASSERT_EQ(actual, expected);
	}

	TEST(summary, valid) {
		std::vector<form> forms{
		    {.key = "DRA"s, .state{}},
		    {.key = "RCA"s, .state{}},
		    {.key = "RCA"s, .state{}},
		};

		auto& DRA = forms[0].state;
		auto& RCA_1 = forms[1].state;
		auto& RCA_2 = forms[2].state;

		DRA.insert(var::insurance_total, 2000_PLN);
		DRA.insert(var::tax_total, 1500'100'900_PLN);

		RCA_1.insert(var::insured.first, "NAME"s);
		RCA_1.insert(var::insured.last, "SURNAME"s);
		RCA_1.insert(var::salary.net, 3.14159_PLN);

		RCA_2.insert(var::insured.first, "JOHN"s);
		RCA_2.insert(var::insured.last, "SMITH"s);
		RCA_2.insert(var::salary.net, 3141.59_PLN);

		auto const summary = gather_summary_data(forms);
		::testing::internal::CaptureStdout();
		print_summary(summary);
		auto const actual = ::testing::internal::GetCapturedStdout();
		auto const expected = R"(-- payments:
   - NAME SURNAME:            3.14 zł
   - JOHN SMITH:           3141.59 zł
   - ZUS:                  2000.00 zł
   - Urząd Skarbowy: 1500100900.00 zł
   sum total =       1500106044.73 zł
)"sv;
		ASSERT_EQ(actual, expected);
	}
}  // namespace quick_dra::testing
