// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <functional>
#include <optional>
#include <quick_dra/base/types.hpp>
#include <span>
#include <string>
#include <string_view>
#include <utility>

namespace quick_dra {
	void comment(std::string_view msg);

	bool get_answer(std::string_view label,
	                std::string_view hint,
	                std::function<bool(std::string&&)> const& validator);

	bool get_enum_answer(
	    std::string_view label,
	    std::span<std::pair<char, std::string_view> const> const& items,
	    std::function<void(char)> const& store_enum,
	    char selected);

	static inline std::string&& as_string(std::string&& value) {
		return std::move(value);
	}

	static inline std::string const& as_string(std::string const& value) {
		return value;
	}

	std::string as_string(insurance_title const& value);
	std::string as_string(ratio const& value);
	std::string as_string(currency const& value);

	template <typename T>
	static inline std::optional<std::string> as_string(
	    std::optional<T> const& value) {
		return value.transform([](T const& value) { return as_string(value); });
	}

	std::optional<std::string> as_string(std::optional<currency> const& value);

	bool get_field_answer(bool ask_questions,
	                      std::string_view label,
	                      std::optional<std::string>& dst,
	                      std::optional<std::string>&& opt,
	                      std::function<bool(std::string&&,
	                                         std::optional<std::string>&,
	                                         bool)> const& validator);
	bool get_field_answer(bool ask_questions,
	                      std::string_view label,
	                      std::optional<insurance_title>& dst,
	                      std::optional<insurance_title>&& opt,
	                      std::function<bool(std::string&&,
	                                         std::optional<insurance_title>&,
	                                         bool)> const& validator);

	bool get_field_answer(bool ask_questions,
	                      std::string_view label,
	                      std::optional<currency>& dst,
	                      std::optional<currency>&& opt,
	                      std::function<bool(std::string&&,
	                                         std::optional<currency>&,
	                                         bool)> const& validator);

	bool get_field_answer(
	    bool ask_questions,
	    std::string_view label,
	    std::optional<ratio>& dst,
	    std::optional<ratio>&& opt,
	    std::function<bool(std::string&&, std::optional<ratio>&, bool)> const&
	        validator);
}  // namespace quick_dra
