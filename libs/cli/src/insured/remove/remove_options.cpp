// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "remove_options.hpp"
#include <optional>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/base/str.hpp>
#include <quick_dra/conv/search.hpp>
#include <string>
#include <utility>
#include <vector>

namespace quick_dra::builtin::insured::remove {
	int get_options(args::parser& parser, options& out) {
		std::optional<std::string> config_path;
		std::optional<unsigned> position;
		std::optional<std::string> search_keyword;

		parser.arg(config_path, "config")
		    .meta("<path>")
		    .help("select config file; defaults to ~/.quick_dra.yaml");

		parser.arg(position, "pos")
		    .meta("<index>")
		    .help(
		        "1-based position of the insured person to remove from "
		        "config");

		parser.arg(search_keyword, "find")
		    .meta("<keyword>")
		    .help(
		        "first or last name, or a document number to use as a "
		        "search key");

		parser.set<std::false_type>(out.ask_questions, "y")
		    .help("remove record if possible; do not ask additional questions")
		    .opt();

		parser.parse();

		if (!position && !search_keyword) {
			parser.error("one of --pos and --find argument is required");
		}

		if (position && search_keyword) {
			parser.error("only one of --pos and --find is allowed");
		}

		out.path = platform::get_config_path(config_path);
		out.cfg = partial::config::load_partial(out.path);
		if (!out.cfg.insured) {
			out.cfg.insured.emplace();
		}

		if (out.cfg.insured->empty()) {
			fmt::print(stderr, "{}: error: there are no items to remove.\n",
			           parser.program());
			return 1;
		}

		out.found.clear();
		auto const on_error = [&parser](std::string const& msg) {
			parser.error(msg);
		};

		if (position) {
			out.found = search_insured_from_position(
			    *position, *out.cfg.insured, on_error);
		} else {
			out.found = search_insured_from_keyword(*search_keyword,
			                                        *out.cfg.insured, on_error);
		}

		return 0;
	}
}  // namespace quick_dra::builtin::insured::remove
