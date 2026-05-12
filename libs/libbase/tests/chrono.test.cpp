// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <quick_dra/base/chrono.hpp>

namespace quick_dra::testing {
	struct testcase {
		year_month input;
		std::string_view expected;
	};

	class fmt_date : public ::testing::TestWithParam<testcase> {};

	TEST_F(fmt_date, empty_mapping) {
		auto const actual = last_date_or_today<int>(null_month, {});
		ASSERT_EQ(month_today(), actual);
	}

	TEST_P(fmt_date, format) {
		auto const& [month, expected] = GetParam();
		auto const actual = quick_dra::fmt_date(month);
		ASSERT_EQ(expected, actual);
	}

	static constexpr testcase tests[] = {
	    {0y / 1, "\"default\" month"sv}, {0y / 2, "0000/02"sv},           {1992y / 1, "January 1992"sv},
	    {2001y / 2, "February 2001"sv},  {1978y / 3, "March 1978"sv},     {1746y / 4, "April 1746"sv},
	    {1997y / 5, "May 1997"sv},       {3000y / 6, "June 3000"sv},      {1410y / 7, "July 1410"sv},
	    {2026y / 8, "August 2026"sv},    {1993y / 9, "September 1993"sv}, {1900y / 10, "October 1900"sv},
	    {1992y / 11, "November 1992"sv}, {1963y / 12, "December 1963"sv}, {1985y / 15, "1985/15"sv},
	};

	INSTANTIATE_TEST_SUITE_P(test, fmt_date, ::testing::ValuesIn(tests));
}  // namespace quick_dra::testing
