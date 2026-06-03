// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <clocale>
#include <limits>
#include <quick_dra/base/types.hpp>
#include <string>
#include <string_view>

namespace quick_dra::locale {
	static constexpr auto nbsp = std::string_view{"\xC2\xA0", 2};
	struct symbol_decorator {
		struct number_view {
			std::string_view sign{};

			bool cs_precedes = false;
			bool sep_by_space = false;
			unsigned sign_pos = 1;

			std::string format(std::string_view symbol,
			                   std::string_view space,
			                   std::string_view value,
			                   bool was_neg) const;
			static number_view from(std::string_view sign, bool cs_precedes, bool sep_by_space, int sign_pos);
		};

		std::string_view symbol{};
		std::string_view space{};
		number_view positive{};
		number_view negative{};

		std::string format(std::string_view value, bool was_neg) const {
			return (was_neg ? negative : positive).format(symbol, space, value, was_neg);
		}

		static symbol_decorator from(std::lconv const* conv, std::string_view space = nbsp);
		static symbol_decorator from_locale(std::string_view space = nbsp) { return from(std::localeconv(), space); }
		static symbol_decorator symmetric(std::string_view symbol,
		                                  std::string_view space,
		                                  std::string_view positive_sign,
		                                  std::string_view negative_sign,
		                                  bool cs_precedes,
		                                  bool sep_by_space,
		                                  int sign_pos);
	};

	struct number_grouping {
		std::string_view decimal_point{};
		std::string_view group_separator{};
		std::string_view grouping{};

		static constexpr auto stop_repeating_blocks = std::numeric_limits<char>::max();

		std::string group(std::string_view number, std::string_view fraction = {}) const;
		std::string group(long long value, unsigned denominator = 1) const;
		template <typename Tag, intmax_t Den>
		std::string group(fixed_point<Tag, Den> const& fixed) const {
			return group(fixed.value, static_cast<size_t>(Den));
		}

		static number_grouping monetary_from(std::lconv const* conv);
		static number_grouping numeric_from(std::lconv const* conv);
		static number_grouping monetary_from_locale() { return monetary_from(std::localeconv()); }
		static number_grouping numeric_from_locale() { return numeric_from(std::localeconv()); }
	};

	struct formatter {
		symbol_decorator decorator{};
		number_grouping grouping{};

		std::string format(long long value, unsigned denominator = 1) const;

		template <typename Tag, intmax_t Den>
		std::string format(fixed_point<Tag, Den> const& fixed) const {
			return format(fixed.value, static_cast<size_t>(Den));
		}

		static formatter monetary_from(std::lconv const* conv, std::string_view space = nbsp);
		static formatter monetary_from_locale(std::string_view space = nbsp) {
			return monetary_from(std::localeconv(), space);
		}

		static formatter percent_with(number_grouping const& grouping);
		static formatter percent_from(std::lconv const* conv) {
			return percent_with(number_grouping::numeric_from(conv));
		}
		static formatter percent_from_locale() { return percent_from(std::localeconv()); }

		static formatter PLN_with(number_grouping const& grouping, std::string_view space = nbsp);
		static formatter PLN_from(std::lconv const* conv, std::string_view space = nbsp) {
			return PLN_with(number_grouping::monetary_from(conv), space);
		}
		static formatter PLN_from_locale(std::string_view space = nbsp) { return PLN_from(std::localeconv(), space); }

		template <intmax_t Den>
		static std::string from_system(currency_base<Den> const& curr, std::string_view space = nbsp) {
			return PLN_from_locale(space).format(curr.value, static_cast<size_t>(Den));
		}

		static std::string from_system(percent const& per) {
			return percent_from_locale().format(per.value, static_cast<size_t>(percent::den));
		}
	};

	template <intmax_t Den>
	static inline std::string from_system(currency_base<Den> const& curr, std::string_view space = nbsp) {
		return formatter::from_system(curr, space);
	}

	static inline std::string from_system(percent const& per) { return formatter::from_system(per); }
}  // namespace quick_dra::locale
