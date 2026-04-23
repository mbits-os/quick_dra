// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <map>
#include <quick_dra/gui/options/debug.hpp>
#include <quick_dra/gui/options/options.hpp>
#include <utility>

namespace quick_dra {
	namespace {
		static constexpr auto dbg_flag_framed = "framed"sv;

		static constexpr debug_option known_options[] = {
		    {
		        .name = dbg_option::app_directory,
		        .meta = "<dir>"sv,
		        .description = "select the directory to serve; defaults to builtin tarball"sv,
		    },
		};

		static constexpr debug_flag known_flags[] = {
#ifdef GUI_DEBUG_FRAMELESS
		    {
		        .name = dbg_flag::frameless,
		        .description = "show the application without OS frame"sv,
		    },
		    {
		        .name = dbg_flag_framed,
		        .target = dbg_flag::frameless,
		        .description = "show the application with OS frame",
		        .default_value = false,
		    },
#endif
		    {
		        .name = dbg_flag::browser,
		        .description = "show the application in a browser instead of a WebView",
		        .implies = "framed"sv,
		        .default_value = false,
		        .select_value = true,
		    },
		    {
		        .name = dbg_flag::devtools,
		        .description = "enable DevTools inside WebView, even on Release",
		        .opposite = "disable DevTools inside WebView, even on Debug",
		    },
		};
	}  // namespace

	std::optional<bool> debug_flag_map::get_default(std::string_view key) const noexcept {
		auto it = defaults_.find(key);
		if (it == defaults_.end()) {
			return std::nullopt;
		}

		return it->second;
	}

	void debug_options::postproc(debug_suite const& suite) {
		auto const& [known_options, known_flags, _] = suite;

		std::map<std::string_view, bool> defaults = {};
		for (auto const& flag : known_flags) {
			defaults[flag.name] = flag.default_value;
		}
		flags.set_defaults(std::move(defaults));

		for (auto const& option : known_options) {
			if (!option.enabled || option.implies.empty()) {
				continue;
			}

			if (options.get(option.name)) {
				handle_implies(option.implies, suite);
			}
		}
		for (auto const& flag : known_flags) {
			if (!flag.enabled || flag.implies.empty()) {
				continue;
			}

			auto const opt = flags.get_maybe(flag.flag_name());
			if (opt && *opt == flag.select_value) {
				handle_implies(flag.implies, suite);
			}
		}
	}

	void debug_options::handle_implies(comma_separated_string_view dbg_implies, debug_suite const& suite) {
		for (auto implies : split_sv(dbg_implies, ','_sep)) {
			auto neg = false;
			if (implies.starts_with("no-"sv)) {
				neg = true;
				implies = implies.substr(3);
			}

			for (auto const& implied : suite.known_flags) {
				if (!implied.enabled || implied.name != implies) {
					continue;
				}
				flags.set(implied.flag_name(), neg ? !implied.select_value : implied.select_value);
			}
		}
	}

	debug_suite get_debug_suite() noexcept {
		return {
		    .known_options = known_options,
		    .known_flags = known_flags,
#ifndef GUI_DEBUG
		    .enabled = false,
#endif
		};
	}
}  // namespace quick_dra
