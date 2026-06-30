// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QWidget>
#include <concepts>
#include <format>

namespace quick_dra::gui {
	template <typename T1, typename T2>
	concept ratio_different_from = std::ratio_not_equal_v<T1, T2>;
	template <typename T1, typename T2>
	concept same_ratio_as = std::ratio_equal_v<T1, T2>;

	template <typename Ratio>
	struct Length {
		using ratio = typename Ratio::type;
		qreal value{};

		constexpr Length() = default;
		explicit constexpr Length(qreal value) : value{value} {}
		template <same_ratio_as<Ratio> Ratio2>
		explicit constexpr Length(Length<Ratio2> const& len) : value{value} {}
		template <ratio_different_from<Ratio> Ratio2>
		explicit constexpr Length(Length<Ratio2> const& len) {
			using TransposeRatio =
			    typename std::ratio<Ratio2::type::num * ratio::den, Ratio2::type::den * ratio::num>::type;
			value = len.value * TransposeRatio::num / TransposeRatio::den;
		}

		constexpr auto operator<=>(Length const& len) const noexcept = default;

		template <ratio_different_from<Ratio> Ratio2>
		constexpr auto operator<=>(Length<Ratio2> const& len) const noexcept {
			return value <=> Length{len}.value;
		}

		template <ratio_different_from<Ratio> Ratio2>
		constexpr bool operator==(Length<Ratio2> const& len) const noexcept {
			return value == Length{len}.value;
		}

		constexpr Length operator-() const noexcept { return Length{-value}; }

		template <typename Ratio2>
		inline constexpr Length operator+(Length<Ratio2> const& rhs) const noexcept {
			return Length{value + Length{rhs}.value};
		}

		template <typename Ratio2>
		inline constexpr Length operator-(Length<Ratio2> const& rhs) const noexcept {
			return Length{value - Length{rhs}.value};
		}

		inline constexpr Length operator*(std::integral auto lhs) const noexcept { return Length<Ratio>{value * lhs}; }

		inline constexpr Length operator*(std::floating_point auto lhs) const noexcept {
			return Length<Ratio>{value * lhs};
		}
	};

	template <typename Ratio>
	inline constexpr Length<Ratio> operator*(std::integral auto lhs, Length<Ratio> const& rhs) noexcept {
		return rhs * lhs;
	}

	template <typename Ratio>
	inline constexpr Length<Ratio> operator*(std::floating_point auto lhs, Length<Ratio> const& rhs) noexcept {
		return rhs * lhs;
	}

	using Inch = Length<std::ratio<1>>;
	using CSSPixel = Length<std::ratio<1, 96>>;
	using Point = Length<std::ratio<1, 72>>;

#define DECL_UDL(TYPE, NAME)                                                                               \
	inline constexpr TYPE operator""_##NAME(long double value) { return TYPE{static_cast<qreal>(value)}; } \
	inline constexpr TYPE operator""_##NAME(long long unsigned value) { return TYPE{static_cast<qreal>(value)}; }
	DECL_UDL(Inch, in)
	DECL_UDL(CSSPixel, px)
	DECL_UDL(Point, pt)

	class DevicePixelScale {
	public:
		explicit DevicePixelScale(auto logDpiX) { updateScale(logDpiX); }
		bool updateScale(qreal logDpiX) {
			if (logDpiX == logicalDpiX_) return false;
			logicalDpiX_ = logDpiX;
			dpScale_ = logDpiX / CSSPixel::ratio::den;
			return true;
		}
		qreal dpScale() const noexcept { return dpScale_; }
		template <typename Ratio>
		qreal toDeviceF(Length<Ratio> const& len) const noexcept {
			return CSSPixel{len}.value * dpScale_;
		}
		template <typename Ratio>
		int toDevice(Length<Ratio> const& len) const noexcept {
			return qRound(toDeviceF(len));
		}

	private:
		qreal logicalDpiX_{CSSPixel::ratio::den};
		qreal dpScale_{1};
	};
}  // namespace quick_dra::gui

namespace std {
	template <typename Ratio>
	struct formatter<quick_dra::gui::Length<Ratio>> : formatter<std::string_view> {
		template <typename FormatContext>
		auto format(quick_dra::gui::Length<Ratio> const& value, FormatContext& ctx) const {
			using base = formatter<std::string_view>;
			using namespace quick_dra::gui;
			if constexpr (std::ratio_equal_v<Ratio, std::ratio<1>>) {
				return base::format(std::format("{}in", value.value), ctx);
			} else if constexpr (std::ratio_equal_v<Ratio, CSSPixel::ratio>) {
				return base::format(std::format("{}px", value.value), ctx);
			} else if constexpr (std::ratio_equal_v<Ratio, Point::ratio>) {
				return base::format(std::format("{}pt", value.value), ctx);
			} else {
				return base::format(std::format("{}[{}/{}]in", value.value, Ratio::num, Ratio::den), ctx);
			}
		}
	};
}  // namespace std
