// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <args_parser.hpp>
#include <filesystem>
#include <quick_dra/conv/conversation.hpp>
#include <quick_dra/models/types.hpp>
#include <variant>
#include <vector>

namespace quick_dra::builtin::insured::edit {
	struct conversation : quick_dra::conversation<partial::insured_t>,
	                      arg_parser {
		std::filesystem::path path;
		std::variant<unsigned, std::string> search_term{};

		conversation(std::string_view tool_name,
		             args::arglist arguments,
		             std::string_view description);

		void parse_args();

		void check_required();
	};
}  // namespace quick_dra::builtin::insured::edit
