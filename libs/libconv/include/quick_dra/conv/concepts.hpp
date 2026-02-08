// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace quick_dra::concepts {
	template <typename T, typename Arg>
	concept Validator = requires(T const& call,
	                             std::string&& src,
	                             std::optional<Arg>& tgt,
	                             bool ask_questions) {
		{ call(std::move(src), tgt, ask_questions) } -> std::same_as<bool>;
	};  // NOLINT(readability/braces)

	template <typename T, typename Selector, typename Arg>
	concept SelectablePerson = requires(T& person, Selector const& call) {
		{ call(person) } -> std::same_as<std::optional<Arg>&>;
	};  // NOLINT(readability/braces)

	template <typename T, typename Arg>
	concept Selector = requires() {
		requires SelectablePerson<typename T::person_type, T, Arg>;
	};  // NOLINT(readability/braces)

	template <typename T, typename Arg>
	concept FieldPolicy = requires() {
		requires Validator<typename T::validator_type, Arg>;
		requires Selector<typename T::selector_type, Arg>;
		requires std::same_as<typename T::value_type, Arg>;
	};  // NOLINT(readability/braces)

	template <typename T>
	concept AnyFieldPolicy = requires() {
		typename T::value_type;
		requires FieldPolicy<T, typename T::value_type>;
	};  // NOLINT(readability/braces)

	template <typename T>
	concept FieldPolicyWithArgFlags = requires(T const& policy) {
		requires AnyFieldPolicy<T>;
		{ policy.arg_flag } -> std::convertible_to<std::string_view>;
	};  // NOLINT(readability/braces)

	template <typename T>
	concept EnumKeyEnabled = requires() {
		{ T::enum_key } -> std::convertible_to<char>;
	};  // NOLINT(readability/braces)
}  // namespace quick_dra::concepts

namespace quick_dra {
	using namespace concepts;
}
