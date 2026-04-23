// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <map>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/base/str.hpp>
#include <quick_dra/gui/options/dbg_types.hpp>
#include <quick_dra/gui/options/options.hpp>
#include <string>
#include <utility>

namespace quick_dra {
	namespace {
		auto set_option(std::string_view name, debug_map<std::string>& tgt) {
			return [name, &tgt](std::string const& value) { tgt.set(name, value); };
		}

		auto set_flag(debug_flag const& flag, debug_map<bool>& tgt, bool value) {
			return [&flag, &tgt, value]() { tgt.set(flag.flag_name(), value); };
		}
	}  // namespace

	std::string ui_config::to_string() {
		if (!cfg) {
			cfg = partial::config::load_partial(cfg_path, false);
		}
		return webapp::config::from(*cfg).store();
	}

	options options::parse(args::args_view const& arguments, debug_suite const& suite) {
		auto const& [known_options, known_flags, flags_enabled] = suite;

		std::optional<std::string> config_path;
		debug_options debug{};

		args::null_translator tr{};
		args::parser parser{"show a GUI for configuration and KEDU generation"s, arguments, &tr};

		if (flags_enabled) {
			size_t debug_items = 0;
			for (auto const& option : known_options) {
				if (!option.enabled) continue;

				++debug_items;
			}

			for (auto const& flag : known_flags) {
				if (!flag.enabled) continue;

				++debug_items;
			}

			if (debug_items) {
				parser.usage("qdra-gui [-h] [--config <path>] [--debug:<flag> ...]"sv);
			}
		}

		parser.arg(config_path, "config").meta("<path>").help("select config file; defaults to ~/.quick_dra.yaml");

		if (flags_enabled) {
			for (auto const& option : known_options) {
				if (!option.enabled) continue;

				auto const name = fmt::format("debug:{}", option.name);
				auto builder = parser.custom(set_option(option.name, debug.options), name);
				builder.help(as_str(option.description)).opt();
				if (!option.meta.empty()) builder.meta(as_str(option.meta));
			}

			for (auto const& flag : known_flags) {
				if (!flag.enabled) continue;

				auto const name = fmt::format("debug:{}", flag.name);
				parser.custom(set_flag(flag, debug.flags, flag.select_value), name)
				    .help(as_str(flag.description))
				    .opt();

				if (!flag.opposite.empty()) {
					auto const neg_name = fmt::format("debug:no-{}", flag.name);
					parser.custom(set_flag(flag, debug.flags, !flag.select_value), neg_name)
					    .help(as_str(flag.opposite))
					    .opt();
				}
			}
		}

		parser.parse();
		debug.postproc(suite);

		return {
		    .cfg_path = platform::get_config_path(config_path),
		    .debug = std::move(debug),
		};
	}  // GCOV_EXCL_LINE[WIN32]
};  // namespace quick_dra
