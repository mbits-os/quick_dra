// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <args_parser.hpp>
#include <quick_dra/base/str.hpp>

namespace quick_dra::builtin::insured::remove {
	int handle(std::string_view tool_name,
	           args::arglist arguments,
	           std::string_view description) {
		args::null_translator tr{};
		args::parser parser{as_str(description), {tool_name, arguments}, &tr};
		parser.parse();
		return 0;
	}
}  // namespace quick_dra::builtin::insured::remove
