// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <quick_dra/models/types.hpp>

#include <fmt/format.h>
#include <array>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/base/str.hpp>
#include <quick_dra/conv/args_parser.hpp>
#include <quick_dra/gui/vfs.hpp>
#include <quick_dra/version.hpp>
#include <span>
#include <webui.hpp>

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

using namespace std::literals;

namespace quick_dra {
	class ui_config {
	public:
		explicit ui_config(std::filesystem::path const& cfg_path) : cfg_path{cfg_path} {};

		std::string to_string();

	private:
		std::filesystem::path cfg_path{};
		std::optional<partial::config> cfg{};
	};

	struct options {
		std::filesystem::path cfg_path{};
		bool transparent{};
		std::optional<bool> devtools{};

		static options parse(args::args_view const& arguments) {
			std::optional<std::string> config_path;
			bool transparent{true};
			std::optional<bool> devtools{};

			args::null_translator tr{};
			args::parser parser{"show a GUI for configuration and KEDU generation"s, arguments, &tr};

			parser.arg(config_path, "config").meta("<path>").help("select config file; defaults to ~/.quick_dra.yaml");
			parser.set<std::false_type>(transparent, "no-transparent")
			    .help("start the application window without transparency")
			    .opt();
			parser.set<std::true_type>(transparent, "transparent")
			    .help("start the application window with transparency")
			    .opt();
			parser.set<std::false_type>(devtools, "no-devtools").help("disable DevTools, even on Debug").opt();
			parser.set<std::true_type>(devtools, "devtools").help("enable DevTools, even on Release").opt();

			parser.parse();

			return {
			    .cfg_path = platform::get_config_path(config_path),
			    .transparent = transparent,
			    .devtools = devtools,
			};
		}  // GCOV_EXCL_LINE[WIN32]
	};

	std::string ui_config::to_string() {
		if (!cfg) {
			cfg = partial::config::load_partial(cfg_path, false);
		}
		return webapp::config::from(*cfg).store();
	}
};  // namespace quick_dra
namespace quick_dra::handlers {
	void bind(webui::window& win, ui_config& cfg) {
		win.bind("minimize", [](webui::window::event* e) { e->get_window().minimize(); });
		win.bind("maximize", [](webui::window::event* e) { e->get_window().maximize(); });
		win.bind("close_win", [](webui::window::event* e) { e->get_window().close(); });
		win.bind("get_version", [](webui::window::event* e) { e->return_string(version::ui); });
		win.bind("get_config", [&cfg](webui::window::event* e) { e->return_string(cfg.to_string()); });
	}
}  // namespace quick_dra::handlers

namespace quick_dra {
	namespace {
		void setup_window(webui::window& win, ui_config& cfg, bool transparent) {
			handlers::bind(win, cfg);

			if (transparent) {
				win.set_frameless(true);
				win.set_transparent(true);
				win.set_resizable(false);
			}

			win.set_size(800, 1200);
			win.set_center();
			win.set_file_handler(gui::virtual_filesystem::global_handler);
		}
	}  // namespace
}  // namespace quick_dra

int gui_tool([[maybe_unused]] args::args_view const& arguments) {
	using namespace quick_dra;

	auto const opts = options::parse(arguments);
	ui_config cfg{opts.cfg_path};

	gui::virtual_filesystem::install_global_data();
	webui::window win{};
	setup_window(win, cfg, opts.transparent);

	win.show_wv("index.html"sv);

#ifdef _WIN32
	auto const hwnd = static_cast<HWND>(win.get_hwnd());
	auto const icon = LoadIconW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(100));
	SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)icon);
	SendMessageW(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
#endif

	if (opts.devtools) {
		win.set_wv_devtools_available(*opts.devtools);
	}

	win.run("refresh()"sv);

	webui::wait();
	return 0;
}
