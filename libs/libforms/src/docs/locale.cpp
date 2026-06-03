// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <array>
#include <format>
#include <quick_dra/docs/locale.hpp>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using namespace std::literals;

namespace quick_dra::locale {
	namespace {
		std::string_view decimal_or_dot(std::string_view from_locale) {
			return from_locale.empty() ? "."sv : from_locale;
		}

		auto clip_sign_pos(int sign_pos) {
			if (sign_pos < 0 || sign_pos > 4) sign_pos = 1;
			return static_cast<unsigned>(sign_pos);
		}

		std::string decorate_number(std::string_view symbol,
		                            std::string_view space,
		                            std::string_view sign,
		                            std::string_view value,
		                            bool use_parens,
		                            bool was_neg,
		                            size_t format_index) {
			if (use_parens) {
				if (was_neg) {
					return std::format("({})", decorate_number(symbol, space, ""sv, value, false, false, format_index));
				} else {
					return decorate_number(symbol, space, ""sv, value, false, false, format_index);
				}
			}

#define SIGN "{0}"
#define QUANTITY "{1}"
#define SPACE "{2}"
#define SYMBOL "{3}"

			static constexpr auto number_formats = std::array{
			    SIGN QUANTITY SYMBOL ""sv, /* The sign string precedes the quantity and currency_symbol.*/
			    QUANTITY SYMBOL SIGN ""sv, /* The sign string succeeds the quantity and currency_symbol.*/
			    QUANTITY SIGN SYMBOL ""sv, /* The sign string immediately precedes the currency_symbol.*/
			    QUANTITY SYMBOL SIGN ""sv, /* The sign string immediately succeeds the currency_symbol.*/

			    SIGN QUANTITY SPACE SYMBOL ""sv,  //
			    QUANTITY SPACE SYMBOL SIGN ""sv,  //
			    QUANTITY SPACE SIGN SYMBOL ""sv,  //
			    QUANTITY SPACE SYMBOL SIGN ""sv,  //

			    SIGN SYMBOL QUANTITY ""sv,  //
			    SYMBOL QUANTITY SIGN ""sv,  //
			    SIGN SYMBOL QUANTITY ""sv,  //
			    SYMBOL SIGN QUANTITY ""sv,  //

			    SIGN SYMBOL SPACE QUANTITY ""sv,  //
			    SYMBOL SPACE QUANTITY SIGN ""sv,  //
			    SIGN SYMBOL SPACE QUANTITY ""sv,  //
			    SYMBOL SIGN SPACE QUANTITY ""sv,  //
			};
#undef SIGN
#undef QUANTITY
#undef SPACE
#undef SYMBOL

			return std::vformat(number_formats[format_index], std::make_format_args(sign, value, space, symbol));
		}

		std::string group_by(std::string_view number,
		                     std::string_view grouping,
		                     std::string_view group_separator,
		                     size_t additional_size) {
			static constexpr auto stop_repeating_blocks = static_cast<size_t>(number_grouping::stop_repeating_blocks);
			auto last_size = stop_repeating_blocks;
			auto const size = number.size() + additional_size;

			std::vector<std::string_view> chunks{};
			{
				size_t chunk_number = 1;
				auto length = number.size();
				for (auto const group_size_char : grouping) {
					auto const group_size = static_cast<size_t>(group_size_char);
					if (group_size == stop_repeating_blocks) break;
					if (group_size >= length) break;
					length -= group_size;
					chunk_number += 1;
				}
				chunks.reserve(chunk_number);
			}

			for (auto const group_size : grouping) {
				last_size = static_cast<size_t>(group_size);
				if (last_size == stop_repeating_blocks) break;
				if (last_size >= number.size()) break;

				chunks.push_back(number.substr(number.length() - last_size));
				number = number.substr(0, number.length() - last_size);
			}

			if (last_size != stop_repeating_blocks) {
				while (last_size <= number.size()) {
					chunks.push_back(number.substr(number.length() - last_size));
					number = number.substr(0, number.length() - last_size);
				}
			}

			chunks.push_back(number);

			std::string result{};
			result.reserve(size + (chunks.size() + 1) * group_separator.size());
			for (auto it = chunks.rbegin(); it != chunks.rend(); ++it) {
				if (!result.empty()) result.append(group_separator);
				result.append(*it);
			}

			return result;
		}
	}  // namespace

	std::string symbol_decorator::number_view::format(std::string_view symbol,
	                                                  std::string_view space,
	                                                  std::string_view value,
	                                                  bool was_neg) const {
		static constexpr auto posn_offset = std::array{0, 0, 1, 2, 3};

		auto const format_index =
		    size_t{static_cast<size_t>(cs_precedes) << 3 | static_cast<size_t>(sep_by_space) << 2 |
		           static_cast<size_t>(posn_offset[static_cast<size_t>(sign_pos)])};
		return decorate_number(symbol, space, sign, value, sign_pos == 0, was_neg, format_index);
	}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
// warning: conversion from ‘int’ to ‘signed char:3’ may change value [-Wconversion]
// .sign_pos = clip_sign_pos(conv->p_sign_posn),
//             ~~~~~~~~~~~~~^~~~~~~~~~~~~~~~~~~
#endif  // __GNUC__
	symbol_decorator::number_view symbol_decorator::number_view::from(std::string_view sign,
	                                                                  bool cs_precedes,
	                                                                  bool sep_by_space,
	                                                                  int sign_pos) {
		return {
		    .sign = sign,
		    .cs_precedes = cs_precedes,
		    .sep_by_space = sep_by_space,
		    .sign_pos = clip_sign_pos(sign_pos),
		};
	}
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif  // __GNUC__

	symbol_decorator symbol_decorator::from(std::lconv const* conv, std::string_view space) {
		return {
		    .symbol = conv->currency_symbol,
		    .space = space,
		    .positive = number_view::from(conv->positive_sign, conv->p_cs_precedes != 0, conv->p_sep_by_space != 0,
		                                  conv->p_sign_posn),
		    .negative = number_view::from(conv->negative_sign, conv->n_cs_precedes != 0, conv->n_sep_by_space != 0,
		                                  conv->n_sign_posn),
		};
	}

	symbol_decorator symbol_decorator::symmetric(std::string_view symbol,
	                                             std::string_view space,
	                                             std::string_view positive_sign,
	                                             std::string_view negative_sign,
	                                             bool cs_precedes,
	                                             bool sep_by_space,
	                                             int sign_pos) {
		return {
		    .symbol = symbol,
		    .space = space,
		    .positive = number_view::from(positive_sign, cs_precedes, sep_by_space, sign_pos),
		    .negative = number_view::from(negative_sign, cs_precedes, sep_by_space, sign_pos),
		};
	}

	std::string number_grouping::group(std::string_view number, std::string_view fraction) const {
		auto result =
		    group_by(number, grouping, group_separator, fraction.empty() ? 0 : fraction.size() + decimal_point.size());

		if (!fraction.empty()) {
			result.append(decimal_point);
			result.append(fraction);
		}

		return result;
	}  // GCOV_EXCL_LINE[GCC]

	std::string number_grouping::group(long long value, unsigned denominator) const {
		size_t digits = 0;
		{
			auto den = denominator;
			while (den > 1) {
				den /= 10;
				++digits;
			}
		}

		auto const absolute = std::abs(value);
		auto number = std::format("{}", absolute / denominator);
		auto fraction = digits ? std::format("{:0{}}", absolute % denominator, digits) : ""s;

		return group(number, fraction);
	}

	number_grouping number_grouping::monetary_from(std::lconv const* conv) {
		return {
		    .decimal_point = decimal_or_dot(conv->mon_decimal_point),
		    .group_separator = conv->mon_thousands_sep,
		    .grouping = conv->mon_grouping,
		};
	}

	number_grouping number_grouping::numeric_from(std::lconv const* conv) {
		return {
		    .decimal_point = decimal_or_dot(conv->decimal_point),
		    .group_separator = conv->thousands_sep,
		    .grouping = conv->grouping,
		};
	}

	std::string formatter::format(long long value, unsigned denominator) const {
		auto const number = grouping.group(value, denominator);
		return decorator.format(number, value < 0);
	}

	formatter formatter::monetary_from(std::lconv const* conv, std::string_view space) {
		return {
		    .decorator = symbol_decorator::from(conv, space),
		    .grouping = number_grouping::monetary_from(conv),
		};
	}

	formatter formatter::percent_with(number_grouping const& grouping) {
		return {
		    .decorator = symbol_decorator::symmetric("%"sv, ""sv, ""sv, "-"sv, false, false, 1),
		    .grouping = grouping,
		};
	}

	formatter formatter::PLN_with(number_grouping const& grouping, std::string_view space) {
		return {
		    .decorator = symbol_decorator::symmetric("zł"sv, space, ""sv, "-"sv, false, true, 1),
		    .grouping = grouping,
		};
	}
}  // namespace quick_dra::locale
