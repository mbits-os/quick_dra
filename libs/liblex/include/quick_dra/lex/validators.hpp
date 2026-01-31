// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <chrono>
#include <limits>
#include <string_view>

namespace quick_dra {
	static constexpr auto kInvalidChecksum =
	    std::numeric_limits<unsigned short>::max();

	namespace detail {
		struct tax_id {
			static unsigned short checksum(std::string_view id) noexcept;
		};

		struct social_id {
			static unsigned short checksum(std::string_view id) noexcept;
			static std::chrono::year_month_day get_birthday(
			    std::string_view social_id) noexcept;
		};

		struct id_card {
			static unsigned short checksum(std::string_view id) noexcept;
		};

		struct pl_passport {
			static unsigned short checksum(std::string_view id) noexcept;
		};

		bool checksum_digit_is_valid(unsigned short checksum,
		                             char tested) noexcept;
		bool fix_checksum_digit(unsigned short checksum, char& fixed) noexcept;

		struct checksum_is_a_digit {
			static bool checksum_is_valid(unsigned short checksum,
			                              char tested) noexcept {
				return checksum_digit_is_valid(checksum, tested);
			}
			static bool fix_checksum(unsigned short checksum,
			                         char& fixed) noexcept {
				return fix_checksum_digit(checksum, fixed);
			}
		};

		struct select_last_character {
			template <typename StringLike>
			static auto& select(StringLike&& id) noexcept {
				return id.back();
			}
		};

		template <size_t Index>
		struct select_nth_character {
			template <typename StringLike>
			static auto& select(StringLike&& id) noexcept {
				return id[Index];
			}
		};
	};  // namespace detail

	template <typename ChecksumPolicy,
	          typename SelectorPolicy,
	          typename ComparePolicy = detail::checksum_is_a_digit>
	struct validator_suite : ChecksumPolicy, SelectorPolicy, ComparePolicy {
		using checksum_t = ChecksumPolicy;
		using compare_t = ComparePolicy;
		using selector_t = SelectorPolicy;

		static bool is_valid(std::string_view id) noexcept {
			auto const checksum = checksum_t::checksum(id);
			if (checksum == kInvalidChecksum) return false;
			return compare_t::checksum_is_valid(checksum,
			                                    selector_t::select(id));
		}

		static std::string fix_checksum(std::string_view id);
	};

	using tax_id_validator =
	    validator_suite<detail::tax_id, detail::select_last_character>;
	using social_id_validator =
	    validator_suite<detail::social_id, detail::select_last_character>;
	using id_card_validator =
	    validator_suite<detail::id_card, detail::select_nth_character<3>>;
	using pl_passport_validator =
	    validator_suite<detail::pl_passport, detail::select_nth_character<2>>;
}  // namespace quick_dra
