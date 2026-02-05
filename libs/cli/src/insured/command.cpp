// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <args_parser.hpp>
#include <quick_dra/base/str.hpp>
#include "../builtins.hpp"

namespace quick_dra::builtin::insured {
	namespace {
#define BUILTINS_X_KNOWN(NAME, TOOL, DSCR) {TOOL##sv, DSCR##sv},
		static constexpr help_command insured_commands[] = {
		    INSURED_BUILTINS_X(BUILTINS_X_KNOWN)};
#undef BUILTINS_X_KNOWN

		static constexpr help_group command_groups[] = {
		    {"known commands"sv, insured_commands},
		};
	}  // namespace

	int handle(std::string_view tool_name,
	           args::arglist arguments,
	           std::string_view description) {
		builtin::parser parser{
		    description, {tool_name, arguments}, command_groups};

		return quick_dra::tools::run(parser, insured::tools, tool_name);
	}
}  // namespace quick_dra::builtin::insured
