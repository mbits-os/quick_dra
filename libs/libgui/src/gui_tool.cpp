// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <quick_dra/models/types.hpp>

#include <fmt/format.h>
#include <array>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/base/str.hpp>
#include <quick_dra/conv/args_parser.hpp>
#include <quick_dra/gui/fs/dir.hpp>
#include <quick_dra/gui/fs/virtual.hpp>
#include <quick_dra/gui/options/options.hpp>
#include <quick_dra/version.hpp>
#include <span>
#include <string>
#include <tangle/uri.hpp>
#include <webui.hpp>

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

using namespace std::literals;

namespace quick_dra::handlers {
	void bind(webui::window& win, ui_config& cfg) {
		win.bind("minimize", [](webui::window::event* e) { e->get_window().minimize(); });
		win.bind("maximize", [](webui::window::event* e) { e->get_window().maximize(); });
		win.bind("close_win", [](webui::window::event* e) { e->get_window().close(); });
		win.bind("get_config", [&cfg](webui::window::event* e) { e->return_string(cfg.to_string()); });
	}
}  // namespace quick_dra::handlers

namespace quick_dra {
	namespace {
		void setup_window(webui::window& win, ui_config& cfg, options const& opts) {
			handlers::bind(win, cfg);

			if (opts.debug.flags.get(dbg_flag::frameless)) {
				win.set_frameless(true);
				win.set_transparent(true);
			}

			win.set_resizable(false);
			win.set_size(600, 800);
			win.set_center();
			win.set_file_handler(opts.debug.options.get(dbg_option::app_directory)
			                         ? gui::directory_filesystem::global_handler
			                         : gui::virtual_filesystem::global_handler);
		}
	}  // namespace

	int gui_tool(options const& opts) {
		auto const app_directory = opts.debug.options.get(dbg_option::app_directory);
		ui_config cfg{opts.cfg_path};

		if (app_directory) {
			gui::directory_filesystem::set_global(as_u8v(*app_directory));
		} else {
			gui::virtual_filesystem::install_global_data();
		}
		webui::window win{};
		setup_window(win, cfg, opts);

		if (opts.debug.flags.get(dbg_flag::browser)) {
			win.show_browser(""sv);
		} else {
			win.show_wv(""sv);
		}

		win.navigate([frameless = opts.debug.flags.get(dbg_flag::frameless)] {
			tangle::search_params search{};

			search.add("frameless"s, frameless ? "1"s : "0"s);
			search.add("version"s, as_str(version::ui));

			tangle::uri url{"index.html"sv};
			url.search(search.string());

			return url.string();
		}());

#ifdef _WIN32
		auto const hwnd = static_cast<HWND>(win.get_hwnd());
		auto const icon = LoadIconW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(100));
		SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)icon);
		SendMessageW(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
#endif

		auto const devtools = opts.debug.flags.get_maybe(dbg_flag::devtools);
		if (devtools) {
			win.set_wv_devtools_available(*devtools);
		}

		webui::wait();
		return 0;
	}
}  // namespace quick_dra

int gui_tool(args::args_view const& arguments) { return quick_dra::gui_tool(quick_dra::options::parse(arguments)); }
