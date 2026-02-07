// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <args_parser.hpp>
#include <filesystem>
#include <quick_dra/conv/conversation.hpp>
#include <quick_dra/models/types.hpp>

namespace quick_dra::builtin::payer {
	struct conversation : quick_dra::conversation<partial::payer_t> {
		std::filesystem::path path;

		void parse_args(std::string_view tool_name,
		                args::arglist arguments,
		                std::string_view description);
	};
}  // namespace quick_dra::builtin::payer
