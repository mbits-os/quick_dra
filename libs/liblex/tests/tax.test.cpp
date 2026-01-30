// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <array>
#include <quick_dra/lex/tax.hpp>
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
		std::map<currency, percent> from(
		    std::span<std::pair<currency, percent> const> scale) {
			return {scale.begin(), scale.end()};
		}
	};  // namespace

	struct tax_testcase {
		std::span<std::pair<currency, percent> const> scale{};
		currency taxable_amount{};
		struct {
			currency tax_lowering_amount{};
			currency tax_owed{};
		} expected;

		friend std::ostream& operator<<(std::ostream& out,
		                                tax_testcase const& params) {
			out << "scale: (";
			auto first = true;
			for (auto [amount, rate] : params.scale) {
				if (first)
					first = false;
				else
					out << ", ";
				out << fmt::format("over {} zł at {}%", amount, rate);
			}
			out << fmt::format("), salary: {} zł, exp: {} zł",
			                   params.taxable_amount,
			                   params.expected.tax_lowering_amount);
			if (params.expected.tax_owed > currency{}) {
				out << fmt::format(" / {} zł", params.expected.tax_owed);
			}
			return out;
		}
	};

	class tax : public ::testing::TestWithParam<tax_testcase> {};

	TEST_P(tax, lowering_amount) {
		auto const& [scale, _, expected] = GetParam();
		auto const actual = calc_tax_lowering_amount(from(scale));
		ASSERT_EQ(actual, expected.tax_lowering_amount);
	}

	TEST_P(tax, owed) {
		auto const& [scale, salary, expected] = GetParam();
		auto const actual = calc_tax_owed(from(scale), salary);
		ASSERT_EQ(actual, expected.tax_owed);
	}

	static constexpr auto scale_17_32 = std::array{
	    std::pair{currency{30'000'00}, percent{17'00}},
	    std::pair{currency{120'000'00}, percent{32'00}},
	};

	static constexpr auto scale_12_32 = std::array{
	    std::pair{currency{30'000'00}, percent{12'00}},
	    std::pair{currency{120'000'00}, percent{32'00}},
	};

	static constexpr auto scale_17_lowering_amount =
	    currency{425'00};  // 5100 / 12

	static constexpr auto scale_12_lowering_amount =
	    currency{300'00};  // 3600 / 12

	static constexpr tax_testcase tests[] = {
	    {
	        .scale = scale_17_32,
	        .taxable_amount = currency{1600'00},
	        .expected{.tax_lowering_amount = scale_17_lowering_amount},
	    },
	    {
	        .scale = scale_12_32,
	        .taxable_amount = currency{1600'00},
	        .expected{.tax_lowering_amount = scale_12_lowering_amount},
	    },
	    {
	        .scale = scale_12_32,
	        .taxable_amount = currency{10'000'00},
	        .expected{.tax_lowering_amount = scale_12_lowering_amount,
	                  .tax_owed = currency{900'00}},
	    },
	    {
	        .scale = scale_12_32,
	        .taxable_amount = currency{11'000'00},
	        .expected{.tax_lowering_amount = scale_12_lowering_amount,
	                  .tax_owed = currency{1220'00}},
	    },
	    {
	        .scale = scale_12_32,
	        .taxable_amount = currency{11'025'46},
	        .expected{.tax_lowering_amount = scale_12_lowering_amount,
	                  .tax_owed = currency{1228'15}},
	    },
	};

	INSTANTIATE_TEST_SUITE_P(test, tax, ::testing::ValuesIn(tests));
}  // namespace quick_dra::testing
