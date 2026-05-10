// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QSizePolicy>
#include <QString>
#include <QToolButton>
#include <string_view>

namespace quick_dra::gui {
	template <typename T>
	concept is_size_policy = requires() { requires T::is_size_policy; };
	template <typename T>
	concept is_modifier = requires() {
		requires !is_size_policy<T>;
		requires T::is_modifier;
	};

	void restrictToolButton(QWidget* button, int size);

	template <typename Widget>
	    requires requires(Widget* tgt, QString const& text) { tgt->setText(text); }
	inline void setText(Widget* widget, std::string_view text) {
		widget->setText(QString::fromUtf8(text));
	}

	template <typename Widget>
	    requires requires(Widget* tgt, QString const& text) { tgt->setText(text); }
	inline void setText(Widget& widget, std::string_view text) {
		widget.setText(QString::fromUtf8(text));
	}

	struct NullBitsModifier {
		static constexpr bool is_modifier = true;
		static void update(QSizePolicy&) noexcept {}
	};

	struct HeightForWidthModifier {
		struct Runtime {
			static constexpr bool is_modifier = true;
			bool value{};
			void update(QSizePolicy& policy) const noexcept { policy.setHeightForWidth(value); }
		};

		static constexpr bool is_modifier = true;
		static void update(QSizePolicy& policy) noexcept { policy.setHeightForWidth(true); }
		Runtime operator()(bool value) const noexcept { return {.value = value}; }
		Runtime operator()(QSizePolicy const& policy) const noexcept { return {.value = policy.hasHeightForWidth()}; }
	};
	static constexpr HeightForWidthModifier HeightForWidth{};

	struct HorizontalStretchModifier {
		struct Runtime {
			static constexpr bool is_modifier = true;
			int stretchFactor{};
			void update(QSizePolicy& policy) const noexcept { policy.setHorizontalStretch(stretchFactor); }
		};

		Runtime operator()(int stretchFactor) const noexcept { return {.stretchFactor = stretchFactor}; }
	};
	static constexpr HorizontalStretchModifier HorizontalStretch{};
	static consteval HorizontalStretchModifier::Runtime operator""_XStretch(unsigned long long factor) {
		return {.stretchFactor = static_cast<int>(factor)};
	}

	struct VerticalStretchModifier {
		struct Runtime {
			static constexpr bool is_modifier = true;
			int stretchFactor{};
			void update(QSizePolicy& policy) const noexcept { policy.setVerticalStretch(stretchFactor); }
		};

		Runtime operator()(int stretchFactor) const noexcept { return {.stretchFactor = stretchFactor}; }
	};
	static constexpr VerticalStretchModifier VerticalStretch{};
	static consteval VerticalStretchModifier::Runtime operator""_YStretch(unsigned long long factor) {
		return {.stretchFactor = static_cast<int>(factor)};
	}

	template <typename... Mod>
	    requires(is_modifier<Mod> && ...)
	struct CombineModifiers : private Mod... {
		template <typename T>
		using With = CombineModifiers<Mod..., T>;

		CombineModifiers(Mod const&... mod) : Mod{mod}... {}

		static constexpr bool is_modifier = true;
		static constexpr bool is_combine = true;
		void update(QSizePolicy& policy) const noexcept { (static_cast<Mod const*>(this)->update(policy), ...); }
	};

	template <typename Mod1, typename Mod2>
	    requires(is_modifier<Mod1> && is_modifier<Mod2>)
	constexpr inline CombineModifiers<Mod1, Mod2> operator/(Mod1 const& lhs, Mod2 const& rhs) {
		return {lhs, rhs};
	}

	template <QSizePolicy::Policy Horizontal, QSizePolicy::Policy Vertical, typename BitsModifier = NullBitsModifier>
	struct SizePolicy : private BitsModifier {
		static constexpr bool is_size_policy = true;
		static constexpr QSizePolicy::Policy horizontal = Horizontal;
		static constexpr QSizePolicy::Policy vertical = Vertical;
		using bits_modifier = BitsModifier;

		SizePolicy() = default;
		SizePolicy(BitsModifier const& mod) : BitsModifier{mod} {}

		bits_modifier const& modifier() const noexcept { return *this; }

		constexpr operator QSizePolicy() const noexcept {
			QSizePolicy result{horizontal, vertical};
			modifier().update(result);
			return result;
		}

		template <typename Modifier>
		    requires Modifier::is_modifier
		constexpr SizePolicy<Horizontal, Vertical, Modifier> operator/(Modifier const& rhs) const noexcept {
			return {{rhs}};
		}
	};

	enum class PolicyAxis {
		Horizontal,
		Vertical,
	};

	template <PolicyAxis Axis, QSizePolicy::Policy Policy>
	struct SizePolicyAlong {
		template <PolicyAxis Axis2, QSizePolicy::Policy Policy2>
		    requires(Axis != Axis2)
		consteval auto operator/(SizePolicyAlong<Axis2, Policy2>) const noexcept {
			if constexpr (Axis == PolicyAxis::Horizontal) {
				return SizePolicy<Policy, Policy2>{};
			} else {
				return SizePolicy<Policy2, Policy>{};
			}
		}
	};

	template <QSizePolicy::Policy Policy>
	struct WidthPolicy : SizePolicyAlong<PolicyAxis::Horizontal, Policy> {};

	template <QSizePolicy::Policy Policy>
	struct HeightPolicy : SizePolicyAlong<PolicyAxis::Vertical, Policy> {};

	static constexpr auto WidthFixed = WidthPolicy<QSizePolicy::Fixed>{};
	static constexpr auto WidthMinimum = WidthPolicy<QSizePolicy::Minimum>{};
	static constexpr auto WidthMaximum = WidthPolicy<QSizePolicy::Maximum>{};
	static constexpr auto WidthPreferred = WidthPolicy<QSizePolicy::Preferred>{};
	static constexpr auto WidthMinimumExpanding = WidthPolicy<QSizePolicy::MinimumExpanding>{};
	static constexpr auto WidthExpanding = WidthPolicy<QSizePolicy::Expanding>{};
	static constexpr auto WidthIgnored = WidthPolicy<QSizePolicy::Ignored>{};

	static constexpr auto HeightFixed = HeightPolicy<QSizePolicy::Fixed>{};
	static constexpr auto HeightMinimum = HeightPolicy<QSizePolicy::Minimum>{};
	static constexpr auto HeightMaximum = HeightPolicy<QSizePolicy::Maximum>{};
	static constexpr auto HeightPreferred = HeightPolicy<QSizePolicy::Preferred>{};
	static constexpr auto HeightMinimumExpanding = HeightPolicy<QSizePolicy::MinimumExpanding>{};
	static constexpr auto HeightExpanding = HeightPolicy<QSizePolicy::Expanding>{};
	static constexpr auto HeightIgnored = HeightPolicy<QSizePolicy::Ignored>{};

	static constexpr auto FixedSize = WidthFixed / HeightFixed;
	static constexpr auto TakeHeight = WidthFixed / HeightPreferred;
	static constexpr auto TakeWidth = WidthPreferred / HeightFixed;
	static constexpr auto TakeAll = WidthPreferred / HeightPreferred;
	static constexpr auto TakeMaximumHeight = WidthFixed / HeightMaximum;
}  // namespace quick_dra::gui
