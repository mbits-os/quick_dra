// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include "commands.hpp"

namespace quick_dra::builtin {
	using namespace std::literals;

#define BUILTINS_X(X)                          \
	X(xml, "xml", "produce KEDU 5.6 XML file") \
	X(payer, "payer", "manage the payer data in ~/.quick_dra.yaml file")

#define BUILTINS_X_DECL(NAME, TOOL, DSCR)         \
	namespace NAME {                              \
		int handle(std::string_view tool,         \
		           args::arglist args,            \
		           std::string_view description); \
	}
	BUILTINS_X(BUILTINS_X_DECL)
#undef BUILTINS_X_DECL

#define BUILTINS_X_DECL(NAME, TOOL, DSCR) {TOOL##sv, DSCR##sv, NAME::handle},
	static constexpr builtin_tool tools[] = {BUILTINS_X(BUILTINS_X_DECL)};
#undef BUILTINS_X_DECL

}  // namespace quick_dra::builtin
