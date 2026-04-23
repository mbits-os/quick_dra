// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <quick_dra/models/types.hpp>

#include <args/parser.hpp>
#include <concepts>
#include <filesystem>
#include <map>
#include <optional>
#include <quick_dra/gui/options/dbg_types.hpp>
#include <quick_dra/gui/options/debug.hpp>
#include <string>
#include <string_view>

using namespace std::literals;

namespace quick_dra {
	namespace dbg_option {
		static constexpr auto app_directory = "app-directory"sv;
	};  // namespace dbg_option

	namespace dbg_flag {
		static constexpr auto frameless = "frameless"sv;
		static constexpr auto browser = "browser"sv;
		static constexpr auto devtools = "devtools"sv;
	};  // namespace dbg_flag

	class ui_config {
	public:
		explicit ui_config(std::filesystem::path const& cfg_path) : cfg_path{cfg_path} {};

		std::string to_string();

	private:
		std::filesystem::path cfg_path{};
		std::optional<partial::config> cfg{};
	};

	struct options {
		std::filesystem::path cfg_path{};
		debug_options debug{};

		static options parse(args::args_view const& arguments, debug_suite const& suite = get_debug_suite());
	};
};  // namespace quick_dra
