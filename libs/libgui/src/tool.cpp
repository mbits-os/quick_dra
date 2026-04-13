// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <array>
#include <quick_dra/base/str.hpp>
#include <quick_dra/conv/args_parser.hpp>
#include <quick_dra/gui/vfs.hpp>
#include <quick_dra/version.hpp>
#include <span>
#include <webui.hpp>

using namespace std::literals;

namespace quick_dra::handlers {
	void bind(webui::window& win) {
		win.bind("minimize", [](webui::window::event* e) { e->get_window().minimize(); });
		win.bind("maximize", [](webui::window::event* e) { e->get_window().maximize(); });
		win.bind("close_win", [](webui::window::event* e) { e->get_window().close(); });
		win.bind("get_version", [](webui::window::event* e) { e->return_string(version::ui); });
	}
}  // namespace quick_dra::handlers

int gui_tool([[maybe_unused]] args::args_view const& arguments) {
	using namespace quick_dra;
	gui::virtual_filesystem::install_global_data();
	webui::window win{};
	handlers::bind(win);

	if (auto const favicon = gui::virtual_filesystem::get_global().locate("/favicon.svg");
	    favicon && favicon->is_file()) {
		auto const& contents = std::get<gui::file_contents>(*favicon);
		win.set_icon({contents.data(), contents.size()}, "image/svg+xml"sv);
	}

	win.set_frameless(true);
	win.set_transparent(true);
	win.set_resizable(false);

	win.set_size(800, 1200);
	win.set_center();
	win.set_file_handler(gui::virtual_filesystem::global_handler);
	win.show_wv("index.html");
	webui::wait();
	return 0;
}
