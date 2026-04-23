// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <span>
#include <string_view>

namespace quick_dra {
	struct comma_separated_string_view : std::string_view {
		using std::string_view::string_view;
		constexpr comma_separated_string_view() = default;
		constexpr comma_separated_string_view(std::string_view const& value) : std::string_view{value} {}
	};

	struct debug_option {
		std::string_view name{};
		std::string_view meta{};
		std::string_view description{};
		comma_separated_string_view implies{};
		bool enabled{true};
	};

	struct debug_flag {
		std::string_view name{};
		std::string_view target{};
		std::string_view description{};
		std::string_view opposite{};
		comma_separated_string_view implies{};
		bool default_value{true};
		bool select_value{default_value};
		bool enabled{true};

		std::string_view flag_name() const noexcept { return !target.empty() ? target : name; }
	};

	struct debug_suite {
		std::span<debug_option const> known_options{};
		std::span<debug_flag const> known_flags{};
		bool enabled{true};
	};

	debug_suite get_debug_suite() noexcept;
}  // namespace quick_dra
