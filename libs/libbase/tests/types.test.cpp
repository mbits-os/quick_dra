// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <array>
#include <optional>
#include <quick_dra/base/types.hpp>
#include <span>
#include <utility>

namespace quick_dra {
	void PrintTo(currency curr, std::ostream* os) {
		*os << fmt::format("{} zł", curr);
	}

	void PrintTo(percent per, std::ostream* os) {
		*os << fmt::format("{}%", per);
	}
}  // namespace quick_dra

namespace quick_dra::testing {
	namespace {
		using std::literals::operator""sv;
		std::string_view suffix(currency) { return " zł"sv; }
		std::string_view suffix(percent) { return "%"sv; }
	}  // namespace

	template <typename FP>
	struct type_testcase {
		std::string_view stored{};
		std::optional<FP> expected{};

		friend std::ostream& operator<<(std::ostream& out,
		                                type_testcase const& test) {
			return out << '"' << test.stored << "\" -> "
			           << (test.expected ? fmt::format("{}{}", *test.expected,
			                                           suffix(*test.expected))
			                             : "<invalid>"sv);
		}
	};

	class money : public ::testing::TestWithParam<type_testcase<currency>> {};
	class cent : public ::testing::TestWithParam<type_testcase<percent>> {};

	TEST_P(money, parse) {
		auto const& [stored, expected] = GetParam();
		currency actual{};
		auto success = currency::parse(stored, actual);
		ASSERT_EQ(!!expected, success);
		if (expected) {
			ASSERT_EQ(*expected, actual);
		}
	}

	TEST_P(cent, parse) {
		auto const& [stored, expected] = GetParam();
		percent actual{};
		auto success = percent::parse(stored, actual);
		ASSERT_EQ(!!expected, success);
		if (expected) {
			ASSERT_EQ(*expected, actual);
		}
	}

	inline static consteval currency operator""_PLN(unsigned long long value) {
		return currency{static_cast<long long>(value * 100)};
	}

	inline static consteval currency operator""_PLN(long double value) {
		return currency{static_cast<long long>(value * 100 + .5)};
	}

	inline static consteval percent operator""_per(unsigned long long value) {
		return percent{static_cast<long long>(value * 100)};
	}

	inline static consteval percent operator""_per(long double value) {
		return percent{static_cast<long long>(value * 100 + .5)};
	}

	static constinit type_testcase<currency> const good_currency_tests[] = {
	    {"0"sv, 0.0_PLN},
	    {"123"sv, 123_PLN},
	    {"-123"sv, -123_PLN},
	    {"0zł"sv, 0.0_PLN},
	    {"123zł"sv, 123_PLN},
	    {"-123.66zł"sv, -123.66_PLN},
	    {"123,333"sv, 123.33_PLN},
	    {"123,777"sv, 123.78_PLN},
	    {"-123,333"sv, -123.33_PLN},
	    {"-123,777"sv, -123.78_PLN},
	    {"-123,334"sv, -123.33_PLN},
	    {"-123,335"sv, -123.34_PLN},
	    {"0 PLN"sv, 0.0_PLN},
	    {"123 PLN"sv, 123_PLN},
	    {"-123 PLN"sv, -123_PLN},
	    {"1,75 zł"sv, 1.75_PLN},
	    {"4800.56 zł"sv, 4800.56_PLN},
	};

	static constinit type_testcase<currency> const bad_currency_tests[] = {
	    {"zł"sv},    {"1,2.3"sv},    {"1,2,3"sv},
	    {"1.2.3"sv}, {"-123 USD"sv}, {"0pln"sv},
	};

	static constinit type_testcase<percent> const good_percent_tests[] = {
	    {"0"sv, 0.0_per},       {"123"sv, 123_per},
	    {"-123"sv, -123_per},   {"0%"sv, 0.0_per},
	    {"123%"sv, 123_per},    {"-123.66%"sv, -123.66_per},
	    {"1,75 %"sv, 1.75_per}, {"4800.56 %"sv, 4800.56_per},
	};

	INSTANTIATE_TEST_SUITE_P(good,
	                         money,
	                         ::testing::ValuesIn(good_currency_tests));

	INSTANTIATE_TEST_SUITE_P(bad,
	                         money,
	                         ::testing::ValuesIn(bad_currency_tests));

	INSTANTIATE_TEST_SUITE_P(good,
	                         cent,
	                         ::testing::ValuesIn(good_percent_tests));

}  // namespace quick_dra::testing
