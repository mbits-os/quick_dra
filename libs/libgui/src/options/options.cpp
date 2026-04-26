// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <map>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/base/str.hpp>
#include <quick_dra/gui/options/options.hpp>
#include <string>
#include <utility>

namespace quick_dra {
	options options::parse(args::args_view const& arguments) {
		std::optional<std::string> config_path;

		args::null_translator tr{};
		args::parser parser{"show a GUI for configuration and KEDU generation"s, arguments, &tr};

		parser.arg(config_path, "config").meta("<path>").help("select config file; defaults to ~/.quick_dra.yaml");

		parser.parse();

		return {
		    .cfg_path = platform::get_config_path(config_path),
		};
	}  // GCOV_EXCL_LINE[WIN32]
};  // namespace quick_dra
