// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <array>
#include <quick_dra/models/model.hpp>
#include <span>
#include <utility>
#include <vector>

namespace quick_dra::testing {
	using testcase = std::pair<maybe_list<compiled_value>, std::string_view>;

	class value_formatter : public ::testing::TestWithParam<testcase> {};

	TEST_P(value_formatter, format) {
		auto const& [arg, expected] = GetParam();
		auto const actual = quick_dra::value_formatter<compiled_value>{}(arg);
		EXPECT_EQ(actual, expected);
	}

	using value_list = std::vector<compiled_value>;
	static testcase const tests[] = {
	    {{}, "<null>"sv},
	    {"string"s, "'string'"sv},
	    {3.14_PLN, "3.14 zł"sv},
	    {98.1_per, "98.10%"sv},
	    {uint_value{1337}, "1337"sv},
	    {addition{.refs = {}}, "()"sv},
	    {addition{.refs = {123}}, "(123)"sv},
	    {addition{.refs = {1, 2, 3}}, "(1 + 2 + 3)"sv},
	    {"first.second.third"_var, "$first.second.third"sv},
	    {value_list{}, "[]"sv},
	    {value_list{"string"s}, "['string']"sv},
	    {
	        value_list{"string"s, 3.14_PLN, 98.1_per, uint_value(1337),
	                   addition{.refs = {1, 2, 3}}, "first.second.third"_var},
	        "['string', 3.14 zł, 98.10%, 1337, (1 + 2 + 3), $first.second.third]"sv,
	    },
	};

	INSTANTIATE_TEST_SUITE_P(test, value_formatter, ::testing::ValuesIn(tests));
}  // namespace quick_dra::testing