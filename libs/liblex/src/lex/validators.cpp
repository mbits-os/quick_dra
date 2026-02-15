// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <array>
#include <cctype>
#include <quick_dra/lex/validators.hpp>
#include "validate/actions.hpp"

namespace quick_dra::detail {
	namespace {
		constexpr auto tax_id_checker =
		    checker::mask<"000000000?">.with<6, 5, 7, 2, 3, 4, 5, 6, 7, 0>().postproc(
		        [](auto sum) {
			        auto const result = sum % 11;
			        if (result > 9) {
				        return kInvalidChecksum;
			        }

			        return static_cast<unsigned short>(result);
		        });

		constexpr auto social_id_checker =
		    checker::mask<"0000000000?">.with<1, 3, 7, 9, 1, 3, 7, 9, 1, 3, 0>().postproc(
		        [](auto sum) { return (10 - (sum % 10)) % 10; });

		constexpr auto id_card_checker =
		    checker::mask<"AAA?00000">.with<7, 3, 1, 0, 7, 3, 1, 7, 3>().postproc(
		        [](auto sum) { return sum % 10; });

		constexpr auto pl_passport_checker =
		    checker::mask<"AA?000000">.with<7, 3, 0, 1, 7, 3, 1, 7, 3>().postproc(
		        [](auto sum) { return sum % 10; });

		template <std::integral T = int>
		T int_parse(std::string_view number) noexcept {
			T result{};
			auto const begin = number.data();
			auto const end = begin + number.size();
			auto const [ptr, ec] = std::from_chars(begin, end, result);
			if (ptr != end || ec != std::errc{}) {
				return 0;
			}
			return result;
		}

		static constexpr auto const centuries =
		    std::array{1900, 2000, 2100, 2200, 1800};
	};  // namespace

	unsigned short tax_id::checksum(std::string_view id) noexcept {
		return tax_id_checker.checksum(id);
	}

	unsigned short social_id::checksum(std::string_view id) noexcept {
		return social_id_checker.checksum(id);
	}

	std::chrono::year_month_day social_id::get_birthday(
	    std::string_view social_id) noexcept {
		if (social_id.length() != 11) {
			return {};
		}

		auto const in_century = int_parse(social_id.substr(0, 2));
		auto const month_and_century =
		    int_parse<unsigned>(social_id.substr(2, 2));
		auto const day = int_parse(social_id.substr(4, 2));

		auto const month = static_cast<int>(month_and_century % 20);
		auto const century_code = month_and_century / 20u;
		auto const century =
		    century_code >= centuries.size() ? 1900 : centuries[century_code];

		auto const result =
		    std::chrono::year{century + in_century} / month / day;
		if (!result.ok()) return {};
		return result;
	}

	unsigned short id_card::checksum(std::string_view id) noexcept {
		return id_card_checker.checksum(id);
	}

	unsigned short pl_passport::checksum(std::string_view id) noexcept {
		return pl_passport_checker.checksum(id);
	}

	bool checksum_digit_is_valid(unsigned short checksum,
	                             char tested) noexcept {
		auto const ch = checksum + '0';
		return tested == static_cast<char>(ch);
	}
}  // namespace quick_dra::detail
