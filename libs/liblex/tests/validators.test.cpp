// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <array>
#include <quick_dra/lex/validators.hpp>
#include <span>
#include <utility>

namespace quick_dra::testing {
	struct id_testcase {
		std::string_view id;
		unsigned short checksum{};

		friend std::ostream& operator<<(std::ostream& out,
		                                id_testcase const& test) {
			out << "id:" << test.id << " checksum:";
			if (test.checksum == kInvalidChecksum) {
				out << "<invalid>";
			} else {
				out << test.checksum;
			}
			return out;
		}
	};

	struct partial_test {
		std::string_view id;
		consteval id_testcase operator()(
		    unsigned short checksum) const noexcept {
			return {.id = id, .checksum = checksum};
		}
	};

	static inline consteval partial_test operator""_test(
	    char const* id,
	    size_t length) noexcept {
		return {.id{id, length}};
	}

	static inline consteval id_testcase operator""_inv(char const* id,
	                                                   size_t length) noexcept {
		return {.id{id, length}, .checksum{kInvalidChecksum}};
	}

	struct birthday_testcase {
		std::string_view social_id;
		std::chrono::year_month_day expected{};

		friend std::ostream& operator<<(std::ostream& out,
		                                birthday_testcase const& test) {
			return out << fmt::format(
			           "id:{} birthday:{:4}-{:02}-{:02}", test.social_id,
			           static_cast<int>(test.expected.year()),
			           static_cast<unsigned>(test.expected.month()),
			           static_cast<unsigned>(test.expected.day()));
		}
	};

	template <typename ValidatorSuite>
	class id_test : public ::testing::TestWithParam<id_testcase>,
	                public ValidatorSuite {
	protected:
		void test_checksum() {
			auto const [id, checksum] = this->GetParam();
			auto actual = ValidatorSuite::checksum(id);
			ASSERT_EQ(actual, checksum);
		}

		void test_valid() {
			auto const [id, checksum] = this->GetParam();
			auto actual = ValidatorSuite::is_valid(id);
			auto expected = checksum != kInvalidChecksum;
			ASSERT_EQ(actual, expected);
		}
	};

#define VALIDATOR_TEST_SUITE(NAME)                    \
	class NAME : public id_test<NAME##_validator> {}; \
	TEST_P(NAME, checksum) { this->test_checksum(); } \
	TEST_P(NAME, valid) { this->test_valid(); }       \
	INSTANTIATE_TEST_SUITE_P(test, NAME, ::testing::ValuesIn(NAME##_tests));

	class social_id_birthday
	    : public ::testing::TestWithParam<birthday_testcase> {};
	TEST_P(social_id_birthday, decode) {
		auto const [id, expected] = GetParam();
		auto const actual = social_id_validator::get_birthday(id);
		ASSERT_EQ(actual, expected);
	}

	using std::literals::operator""sv;
	using std::literals::operator""y;

	static constexpr id_testcase tax_id_tests[] = {
	    ""_inv,           "02070803628"_inv,    "0000000000"_test(0),
	    "1234567890"_inv, "1234563218"_test(8), "7680002466"_test(6),
	};

	static constexpr id_testcase social_id_tests[] = {
	    ""_inv,
	    "1234563218"_inv,
	    "020708036282"_inv,
	    "02070803628"_test(8),
	    "78070707132"_test(2),
	    "00000000000"_test(0),
	    "26211012346"_test(6),
	};

	static constexpr birthday_testcase birthday_tests[] = {
	    {"50871500000"sv, 1850y / 7 / 15}, {"50901500000"sv, 1850y / 10 / 15},
	    {"02070800000"sv, 1902y / 7 / 8},  {"98122400000"sv, 1998y / 12 / 24},
	    {"98222400000"sv, 2098y / 2 / 24}, {"50310100000"sv, 2050y / 11 / 01},
	    {"50471500000"sv, 2150y / 7 / 15}, {"50501500000"sv, 2150y / 10 / 15},
	    {"50671500000"sv, 2250y / 7 / 15}, {"50701500000"sv, 2250y / 10 / 15},
	};

	static constexpr auto null_day = 0y / 0 / 0;

	static constexpr birthday_testcase bad_birthday_tests[] = {
	    {"504715"sv, null_day},
	    {"504715504715"sv, null_day},
	    {"504A1500000"sv, null_day},
	    {"5047A500000"sv, null_day},
	};

	static constexpr id_testcase id_card_tests[] = {
	    ""_inv,
	    "aaa000000"_inv,
	    "000000000"_inv,
	    "AAAAAAAAA"_inv,
	    "AAA000000"_test(0),
	    "ABC523456"_test(5),
	    "ABS123456"_test(1),
	    "ZZZ499999"_test(4),
	};

	static constexpr id_testcase pl_passport_tests[] = {
	    ""_inv,
	    "aa0000000"_inv,
	    "000000000"_inv,
	    "AAAAAAAAA"_inv,
	    "AA0000000"_test(0),
	    "EH0123456"_test(0),
	    "AB4123456"_test(4),
	    "ZZ8999999"_test(8),
	};

	INSTANTIATE_TEST_SUITE_P(test,
	                         social_id_birthday,
	                         ::testing::ValuesIn(birthday_tests));
	INSTANTIATE_TEST_SUITE_P(bad_test,
	                         social_id_birthday,
	                         ::testing::ValuesIn(bad_birthday_tests));

	VALIDATOR_TEST_SUITE(tax_id);
	VALIDATOR_TEST_SUITE(social_id);
	VALIDATOR_TEST_SUITE(id_card);
	VALIDATOR_TEST_SUITE(pl_passport);
}  // namespace quick_dra::testing
