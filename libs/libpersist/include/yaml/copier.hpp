// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <concepts>
#include <vector>

namespace yaml {
	namespace concepts {
		template <typename T>
		struct is_optional : std::false_type {};

		template <typename T>
		struct is_optional<T&> : is_optional<T> {};

		template <typename T>
		struct is_optional<T const> : is_optional<T> {};

		template <typename T>
		struct is_optional<std::optional<T>> : std::true_type {};

		template <typename T>
		concept OptionalType = static_cast<bool>(is_optional<T>{});

		template <typename T>
		concept RequiredType = !OptionalType<T>;

		template <typename Target, typename Source>
		concept ConstructibleValueFrom = requires(Source const& source) {
			{ Target::from(source) } -> std::convertible_to<Target>;
		};  // NOLINT(readability/braces)
	}  // namespace concepts

	template <concepts::RequiredType Same>
	inline void take_from(Same& tgt, Same const& src);

	template <typename T, typename S>
	inline void take_from(std::optional<T>& tgt, std::optional<S> const& src);

	template <typename T, typename S>
	inline void take_from(std::vector<T>& tgt, std::vector<S> const& src);

	template <concepts::RequiredType Source, concepts::ConstructibleValueFrom<Source> Target>
	inline void take_from(Target& tgt, Source const& src) {
		tgt = Target::from(src);
	}

}  // namespace yaml

#include "copier.inl"
