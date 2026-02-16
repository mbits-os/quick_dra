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
		                       FixedPoint& dst,
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
			value += value < 0 ? -0.5 : 0.5;
			dst = FixedPoint{static_cast<long long>(value)};
			return true;
		}
	}  // namespace

	bool currency::parse(std::string_view input, currency& dst) {
		return parse_fixed_point(input, dst, "zł"sv, "PLN"sv);
	}

	bool percent::parse(std::string_view input, percent& dst) {
		return parse_fixed_point(input, dst, "%"sv);
	}

	bool ratio::parse(std::string_view input, ratio& dst) {
		if (input.ends_with('%')) {
			percent p{};
			if (percent::parse(input, p)) {
				dst = ratio::gcd(static_cast<unsigned>(p.value),
				                 static_cast<unsigned>(100 * percent::den));
				return true;
			}
		}

		auto const split = split_sv(input, '/'_sep);
		if (split.size() != 2) {
			dst = {};
			return false;
		}

		unsigned num{};
		unsigned den{};

		if (!from_chars(split[0], num) || !from_chars(split[1], den)) {
			return false;
		}

		dst = ratio::gcd(num, den);
		return true;
	}

	bool insurance_title::parse(std::string_view input, insurance_title& dst) {
		auto const split = split_sv(input, ' '_sep);
		if (split.size() != 3 || split[0].length() != 4 ||
		    split[1].length() != 1 || split[2].length() != 1) {
			dst = {};
			return false;
		}

		for (auto const& view : split) {
			for (auto const ch : view) {
				if (!std::isdigit(static_cast<unsigned char>(ch))) {
					return false;
				}
			}
		}

		auto const& title_code = split[0];
		auto const pension_right =
		    static_cast<unsigned short>(split[1].front() - '0');
		auto const disability_level =
		    static_cast<unsigned short>(split[2].front() - '0');

		dst = {
		    .title_code{title_code.data(), title_code.size()},
		    .pension_right = pension_right,
		    .disability_level = disability_level,
		};
		return true;
	}
}  // namespace quick_dra
