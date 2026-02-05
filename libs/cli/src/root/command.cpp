// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <quick_dra/base/str.hpp>
#include <span>
#include "../builtins.hpp"

namespace quick_dra::builtin {
	namespace {
#define BUILTINS_X_KNOWN(NAME, TOOL, DSCR) {TOOL##sv, DSCR##sv},
		static constexpr help_command root_commands[] = {
		    ROOT_BUILTINS_X(BUILTINS_X_KNOWN)};
#undef BUILTINS_X_KNOWN

		static constexpr help_group command_groups[] = {
		    {"known commands"sv, root_commands},
		};
	}  // namespace
}  // namespace quick_dra::builtin

int tool(args::args_view const& arguments) {
	using namespace quick_dra;

	builtin::root_parser parser{arguments, builtin::command_groups};
	return quick_dra::tools::run(parser, builtin::tools, "qdra"sv);
}
