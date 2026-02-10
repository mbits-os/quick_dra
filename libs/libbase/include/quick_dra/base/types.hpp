// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <fmt/format.h>
#include <charconv>
#include <concepts>
#include <numeric>
#include <string>
#include <vector>

namespace quick_dra {
	static inline bool from_chars(std::string_view view,
	                              std::integral auto& dst) {
		auto const begin = view.data();
		auto const end = begin + view.size();
		auto const [ptr, ec] = std::from_chars(begin, end, dst);
		if (ptr != end || ec != std::errc{}) {
			return false;
		}
		return true;
	}

	template <typename Tag, typename Type>
	struct strong_typedef {
		Type value{};

		strong_typedef() = default;
		explicit strong_typedef(Type v) : value{v} {}
	};

	using uint_value = strong_typedef<class number_tag, unsigned>;

	template <typename Tag, intmax_t Den>
	struct fixed_point {
		using tag_type = Tag;
		static constexpr auto den = Den;
		long long value{};

		constexpr fixed_point() noexcept = default;
		constexpr explicit fixed_point(long long value) noexcept
		    : value{value} {}

		template <intmax_t D2 = 100>
		constexpr fixed_point<Tag, D2> rounded_impl() const noexcept {
			if constexpr (Den < D2) {
				return fixed_point<Tag, D2>{value * Den / D2};
			} else {
				auto const value_f = static_cast<long double>(value) / Den;
				auto const rescaled = value_f * D2 + 0.5l;
				return fixed_point<Tag, D2>{static_cast<long long>(rescaled)};
			}
		}

		constexpr auto operator<=>(fixed_point const&) const noexcept = default;
	};

	template <typename T>
	concept fixed_child = requires(T obj) {
		typename T::tag_type;
		{ T::den } -> std::common_with<intmax_t>;
		{ obj.value } -> std::common_with<long long>;
		requires std::derived_from<T,
		                           fixed_point<typename T::tag_type, T::den>> ||
		             std::same_as<T, fixed_point<typename T::tag_type, T::den>>;
	};  // NOLINT(readability/braces)

	template <fixed_child Value>
	inline constexpr Value operator+(Value const& lhs,
	                                 Value const& rhs) noexcept {
		return Value{lhs.value + rhs.value};
	}

	template <fixed_child Value>
	inline constexpr Value operator-(Value const& lhs,
	                                 Value const& rhs) noexcept {
		return Value{lhs.value - rhs.value};
	}

	template <intmax_t Den = 100'000>
	struct currency_base : fixed_point<class currency_tag, Den> {
		using base = fixed_point<class currency_tag, Den>;

		using base::base;
		constexpr auto operator<=>(currency_base const&) const noexcept =
		    default;
	};

	struct currency;

	struct calc_currency : currency_base<100'00> {
		using currency_base<100'00>::currency_base;

		constexpr inline currency rounded() const noexcept;
		constexpr auto operator<=>(calc_currency const&) const noexcept =
		    default;
	};

	struct currency : currency_base<100> {
		using currency_base<100>::currency_base;

		static bool parse(std::string_view, currency&);

		constexpr calc_currency calc() const noexcept {
			return calc_currency{value * 100};
		}
		constexpr auto operator<=>(currency const&) const noexcept = default;

		constexpr currency operator-() const noexcept {
			return currency{-value};
		}
	};

	inline static consteval currency operator""_PLN(unsigned long long value) {
		return currency{static_cast<long long>(value * 100)};
	}

	inline static consteval currency operator""_PLN(long double value) {
		return currency{static_cast<long long>(value * 100 + .5)};
	}

	static inline constexpr auto minimal_salary = -1_PLN;

	constexpr inline currency calc_currency::rounded() const noexcept {
		return currency{base::template rounded_impl<currency::den>().value};
	}

	struct percent : fixed_point<class percent_tag, 100> {
		using base = fixed_point<class percent_tag, 100>;
		using base::base;

		static bool parse(std::string_view, percent&);

		constexpr percent operator-() const noexcept { return percent{-value}; }
	};

	inline constexpr calc_currency operator*(calc_currency const& amount,
	                                         percent const& percent) noexcept {
		return calc_currency{amount.value * percent.value / 100'00};
	}

	struct ratio {
		unsigned num{};
		unsigned den{};

		constexpr bool operator==(ratio const& rhs) const noexcept {
			return (num * rhs.den) == (rhs.num * den);
		}
		constexpr auto operator<=>(ratio const& rhs) const noexcept {
			return (num * rhs.den) <=> (rhs.num * den);
		}

		static bool parse(std::string_view, ratio&);
		static constexpr ratio gcd(unsigned num, unsigned den) noexcept {
			if (!num || !den) {
				return {};
			}
			auto div = std::gcd(num, den);
			return {.num = num / div, .den = den / div};
		}

		constexpr ratio gcd() const noexcept { return gcd(num, den); }
		constexpr bool valid() const noexcept { return den != 0; }
	};

	static inline constexpr auto full_time = ratio{1u, 1u};

	static_assert(ratio{3, 4} == ratio{6, 8});
	static_assert(ratio{3, 4} > ratio{5, 8});

	struct insurance_title {
		std::string title_code{};
		unsigned short pension_right{};
		unsigned short disability_level{};

		std::vector<std::string> split() const {
			std::vector<std::string> result{3};
			result[0] = title_code;
			result[1] = fmt::to_string(pension_right);
			result[2] = fmt::to_string(disability_level);
			return result;
		}

		auto operator<=>(insurance_title const& rhs) const noexcept = default;

		static bool parse(std::string_view, insurance_title&);
	};

	struct costs_of_obtaining {
		currency local{};
		currency remote{};
	};

	struct contribution {
		currency payer{};
		currency insured{};

		constexpr currency total() const noexcept { return payer + insured; }
	};

	struct rate {
		percent payer{};
		percent insured{};

		constexpr percent total() const noexcept { return payer + insured; }
		constexpr contribution contribution_on(currency amount) const noexcept {
			return contribution_on(amount.calc());
		}
		constexpr contribution contribution_on(
		    calc_currency amount) const noexcept {
			auto const payer_contribution = (amount * payer).rounded();
			auto const insured_contribution = (amount * insured).rounded();
			return {.payer = payer_contribution,
			        .insured = insured_contribution};
		}
	};

#define CONTRIBUTIONS(X)               \
	X(health_insurance, "chorobowe")   \
	X(pension_insurance, "emerytalne") \
	X(disability_insurance, "rentowe") \
	X(accident_insurance, "wypadkowe")
#define CONTRIBUTIONS_EX(X) \
	CONTRIBUTIONS(X)        \
	X(health, "zdrowotne")

	struct contributions {
#define X(NAME, _) contribution NAME{};
		CONTRIBUTIONS(X)
#undef X

		constexpr currency total() const noexcept {
#define X(NAME, _) NAME.total() +
			return CONTRIBUTIONS(X) currency{};
#undef X
		}

		constexpr currency payer() const noexcept {
#define X(NAME, _) NAME.payer +
			return CONTRIBUTIONS(X) currency{};
#undef X
		}

		constexpr currency insured() const noexcept {
#define X(NAME, _) NAME.insured +
			return CONTRIBUTIONS(X) currency{};
#undef X
		}
	};

	struct rates {
#define X(NAME, _) rate NAME{};
		CONTRIBUTIONS_EX(X)
#undef X

		constexpr contributions contribution_on(
		    currency amount) const noexcept {
			return contribution_on(amount.calc());
		}
		constexpr contributions contribution_on(
		    calc_currency amount) const noexcept {
			return {
#define X(NAME, _) .NAME = NAME.contribution_on(amount),
			    CONTRIBUTIONS(X)
#undef X
			};
		}
	};
}  // namespace quick_dra

namespace fmt {
	template <typename Tag, typename Type>
	struct formatter<quick_dra::strong_typedef<Tag, Type>> : formatter<Type> {
		template <typename FormatContext>
		auto format(quick_dra::strong_typedef<Tag, Type> const& value,
		            FormatContext& ctx) const {
			return formatter<Type>::format(value.value, ctx);
		}
	};

	template <typename Tag, intmax_t Den>
	struct formatter<quick_dra::fixed_point<Tag, Den>> : formatter<double> {
		template <typename FormatContext>
		auto format(quick_dra::fixed_point<Tag, Den> const& fp,
		            FormatContext& ctx) const {
			auto hundredth = Den == 100 ? fp.value : fp.rounded_impl().value;
			return formatter<double>::format(
			    static_cast<double>(hundredth) / 100.0, ctx);
		}
	};

	template <>
	struct formatter<quick_dra::currency>
	    : formatter<quick_dra::currency::base> {};

	template <>
	struct formatter<quick_dra::percent> : formatter<quick_dra::percent::base> {
	};
}  // namespace fmt
