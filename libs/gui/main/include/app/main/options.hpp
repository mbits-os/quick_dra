// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <quick_dra/models/types.hpp>

#include <args/parser.hpp>
#include <concepts>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <string_view>

using namespace std::literals;

namespace quick_dra {
	struct exit_on_help {};
	struct options {
		std::filesystem::path cfg_path{};
		std::optional<std::filesystem::path> tax_config_path;

		static options parse(args::args_view const& arguments);
	};
};  // namespace quick_dra
