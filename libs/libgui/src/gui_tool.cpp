// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <quick_dra/models/types.hpp>

#include <fmt/format.h>
#include <array>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/base/str.hpp>
#include <quick_dra/conv/args_parser.hpp>
#include <quick_dra/gui/options/options.hpp>
#include <quick_dra/version.hpp>
#include <span>
#include <string>

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

using namespace std::literals;

namespace quick_dra {
	int gui_tool(options const& opts) {
		auto cfg = partial::config::load_partial(opts.cfg_path, false);

		return 0;
	}
}  // namespace quick_dra

int gui_tool(args::args_view const& arguments) { return quick_dra::gui_tool(quick_dra::options::parse(arguments)); }
