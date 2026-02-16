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

	void PrintTo(ratio r, std::ostream* os) {
		*os << fmt::format("{}/{}", r.num, r.den);
	}

	void PrintTo(insurance_title it, std::ostream* os) {
		*os << fmt::format("{} {} {}", it.title_code, it.pension_right,
		                   it.disability_level);
	}
}  // namespace quick_dra

namespace quick_dra::testing {
	namespace {
		using std::literals::operator""sv;
		using std::literals::operator""s;
		std::string_view suffix(currency) { return " zł"sv; }
		std::string_view suffix(percent) { return "%"sv; }
		std::string_view suffix(auto const&) { return ""sv; }
	}  // namespace

	template <typename Type>
	struct type_testcase {
		std::string_view stored{};
		std::optional<Type> expected{};

		friend std::ostream& operator<<(std::ostream& out,
		                                type_testcase const& test) {
			return out << '"' << test.stored << "\" -> "
			           << (test.expected ? fmt::format("{}{}", *test.expected,
			                                           suffix(*test.expected))
			                             : "<invalid>"sv);
		}
	};

	template <typename Type>
	class type_test : public ::testing::TestWithParam<type_testcase<Type>> {
	protected:
		void parse_test() {
			auto const& [stored, expected] = this->GetParam();
			Type actual{};
			auto success = Type::parse(stored, actual);
			ASSERT_EQ(!!expected, success);
			if (expected) {
				ASSERT_EQ(*expected, actual);
			}
		}
	};

	class money : public type_test<currency> {};
	class cent : public type_test<percent> {};
	class ratios : public type_test<ratio> {};
	class insurance_titles : public type_test<insurance_title> {};

	TEST_P(money, parse) { parse_test(); }
	TEST_P(cent, parse) { parse_test(); }
	TEST_P(ratios, parse) { parse_test(); }
	TEST_P(insurance_titles, parse) { parse_test(); }

	TEST(ratios, compare) {
		ASSERT_GT(ratio(1, 3), ratio(1, 4));
		ASSERT_LT(ratio(2, 3), ratio(3, 4));
	}

	TEST(money, contributions) {
		static constexpr auto r = rate{.payer = 5_per, .insured = -23_per};
		auto const contr = r.contribution_on(100000_PLN);
		ASSERT_EQ(r.total(), -18_per);
		ASSERT_EQ(contr.total(), -18000_PLN);
		ASSERT_EQ(contr.payer, 5000_PLN);
		ASSERT_EQ(contr.insured, -23000_PLN);
	}

	struct currency_ex : public currency {
		using currency::currency;

		constexpr inline calc_currency un_rounded() const noexcept {
			return calc_currency{
			    currency::template rounded_impl<calc_currency::den>().value};
		}
	};

	TEST(money, rounding) {
		auto const tested = 23.567_PLN;
		auto const un_rounded = currency_ex{tested.value}.un_rounded();
		ASSERT_EQ(tested, 23.57_PLN);
		ASSERT_EQ(tested.value, 2357);
		ASSERT_EQ(un_rounded.value, 235700);
		ASSERT_EQ((un_rounded * 13_per).value, 30641);
		ASSERT_EQ((un_rounded * 13_per).rounded(), 3.0641_PLN);
		ASSERT_EQ(((-23.567_PLN).calc() * 13_per).value, -30641);
		ASSERT_EQ(calc_currency(-30641).rounded(), currency(-306));
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

	static constinit type_testcase<ratio> const good_ratio_tests[] = {
	    {"33%"sv, ratio{33, 100}}, {"75%"sv, ratio{3, 4}},
	    {"0/0"sv, ratio{0, 0}},    {"0/1"sv, ratio{0, 1}},
	    {"3/4"sv, ratio{12, 16}},
	};

	static constinit type_testcase<ratio> const bad_ratio_tests[] = {
	    {"123"sv},     {"1/2/3"sv},  {"alpha/beta"sv},
	    {"alpha/1"sv}, {"1/beta"sv}, {"5e3/2"sv},
	};

	static type_testcase<insurance_title> const good_insurance_title_tests[] = {
	    {"0000 0 0"sv, insurance_title{"0000"s, 0, 0}},
	    {"0110 1 5"sv, insurance_title{"0110"s, 1, 5}},
	};

	static constinit type_testcase<insurance_title> const
	    bad_insurance_title_tests[] = {
	        {"0 0 0"sv},    {"00000 0 0"sv}, {"0000 00 0"sv}, {"aaaa 0 0"sv},
	        {"0000 a 0"sv}, {"0000 0 !"sv},  {"0000 0 00"sv},
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

	INSTANTIATE_TEST_SUITE_P(good,
	                         ratios,
	                         ::testing::ValuesIn(good_ratio_tests));

	INSTANTIATE_TEST_SUITE_P(bad, ratios, ::testing::ValuesIn(bad_ratio_tests));

	INSTANTIATE_TEST_SUITE_P(good,
	                         insurance_titles,
	                         ::testing::ValuesIn(good_insurance_title_tests));

	INSTANTIATE_TEST_SUITE_P(bad,
	                         insurance_titles,
	                         ::testing::ValuesIn(bad_insurance_title_tests));

}  // namespace quick_dra::testing
