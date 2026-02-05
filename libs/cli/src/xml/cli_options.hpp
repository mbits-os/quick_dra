// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <args_parser.hpp>
#include <quick_dra/io/options.hpp>

namespace quick_dra::builtin::xml {
	options options_from_cli(args::args_view const& arguments,
	                         std::string_view description);
}  // namespace quick_dra::builtin::xml
