// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace quick_dra {
	void comment(std::string_view msg);

	bool get_answer(std::string_view label,
	                std::string_view hint,
	                std::function<bool(std::string&&)> const& validator);

	bool get_enum_answer(
	    std::string_view label,
	    std::span<std::pair<char, std::string_view> const> const& items,
	    std::function<void(char)> const& store_enum);

	bool get_string_answer(bool ask_questions,
	                       std::string_view label,
	                       std::optional<std::string>& dst,
	                       std::optional<std::string>&& opt,
	                       std::function<bool(std::string&&,
	                                          std::optional<std::string>&,
	                                          bool)> const& validator);
}  // namespace quick_dra
