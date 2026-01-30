// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <charconv>
#include <quick_dra/base/str.hpp>
#include <quick_dra/base/types.hpp>

namespace quick_dra {
	namespace {
		template <size_t... Index>
		std::string_view strip_suffix(
		    std::string_view input,
		    std::string_view suffix,
		    std::same_as<std::string_view> auto... suffixes) {
			if (input.ends_with(suffix)) {
				return strip_sv(input.substr(0, input.size() - suffix.size()));
			}
			if constexpr (sizeof...(suffixes) > 0) {
				return strip_suffix(input, suffixes...);
			} else {
				return input;
			}
		}

		template <typename FixedPoint>
		bool parse_fixed_point(std::string_view input,
		                       FixedPoint& ctx,
		                       std::same_as<std::string_view> auto... suffix) {
			input = strip_suffix(strip_sv(input), suffix...);

			std::string alt_storage{};
			auto const comma = input.find(',');
			auto const has_comma = comma != std::string_view::npos;
			auto const has_dot = input.find('.') != std::string_view::npos;

			if (has_comma && !has_dot) {
				alt_storage = input;
				alt_storage[comma] = '.';
				input = alt_storage;
			}

			double value{};
			auto const begin = input.data();
			auto const end = begin + input.size();

			auto const [ptr, ec] = std::from_chars(begin, end, value);
			if (ptr != end || ec != std::errc{}) {
				return false;
			}

			value *= FixedPoint::den;
			value += 0.5;
			ctx = FixedPoint{static_cast<long long>(value)};
			return true;
		}
	}  // namespace

	bool currency::parse(std::string_view input, currency& ctx) {
		return parse_fixed_point(input, ctx, "zł"sv, "PLN"sv);
	}

	bool percent::parse(std::string_view input, percent& ctx) {
		return parse_fixed_point(input, ctx, "%"sv);
	}
}  // namespace quick_dra
