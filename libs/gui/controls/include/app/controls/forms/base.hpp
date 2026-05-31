// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <concepts>
#include <optional>
#include <string_view>

namespace quick_dra::gui {
	enum class Validation { Ok, Empty, TooShort, TooLong, Invalid };

	template <typename T, typename Target>
	concept MaybeRef = std::same_as<T, Target&> || std::same_as<T, std::optional<Target>&>;

	template <typename T, typename Accessed, typename ValueType>
	concept TargetAccessor = requires(Accessed& object) {
		{ T::getField(object) } -> MaybeRef<ValueType>;
	};

	template <typename T>
	concept Declaration = requires(std::string_view value) {
		requires TargetAccessor<T, typename T::target_type, typename T::value_type>;
		{ T::label } -> std::convertible_to<std::string_view const&>;
		{ T::error_message } -> std::convertible_to<std::string_view const&>;
		{ T::validate(value) } -> std::convertible_to<Validation>;
	};

	template <typename T, typename Parent>
	concept VectorItemDeclaration = requires() {
		requires Declaration<T>;
		requires Declaration<Parent>;
		requires std::same_as<std::vector<typename T::target_type>, typename Parent::value_type>;
	};

	template <typename T>
	concept DocumentKindDeclaration = requires() {
		requires Declaration<T>;
		{ T::enum_key } -> std::convertible_to<std::string_view const&>;
	};
}  // namespace quick_dra::gui
