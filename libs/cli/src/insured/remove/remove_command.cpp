// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <args_parser.hpp>
#include "remove_options.hpp"

#include <fmt/format.h>
#include <fmt/std.h>
#include <cstdio>
#include <iterator>
#include <quick_dra/base/str.hpp>
#include <quick_dra/conv/low_level.hpp>
#include <quick_dra/models/types.hpp>
#include <string_view>
#include <type_traits>
#include <vector>

namespace quick_dra::builtin::insured::remove {
	int handle(std::string_view tool_name,
	           args::arglist arguments,
	           std::string_view description) {
		options opts{};

		{
			args::null_translator tr{};
			args::parser parser{
			    as_str(description), {tool_name, arguments}, &tr};

			auto const ret = get_options(parser, opts);
			if (ret) {
				return ret;
			}
		}

		if (!opts.found.empty()) {
			fmt::print("Found:\n");
		}

		for (auto const index : opts.found) {
			auto const& person = opts.cfg.insured->at(index);
			fmt::print("    #{0}: {1} {2} [{3} {4}]\n", index + 1,
			           person.first_name.value_or("??"),
			           person.last_name.value_or("??"),
			           person.kind.value_or("??"),
			           person.document.value_or("??"));
		}

		if (opts.found.size() > 1) {
			fmt::print(
			    "\n"
			    "This search was ambiguous. Please refine your parameter or "
			    "use --pos to pinpoint the record.\n");
			return 1;
		}

		if (opts.ask_questions) {
			bool remove_allowed = false;
			fmt::print("\n");
			if (!get_yes_no("Do you want to delete this record?", true,
			                remove_allowed)) {
				return 1;
			}

			if (!remove_allowed) {
				fmt::print("OK, I won't\n");
				return 0;
			}
		}

		opts.cfg.insured->erase(
		    std::next(opts.cfg.insured->begin(), opts.found.front()));
		if (!opts.cfg.store(opts.path)) {
			fmt::print(stderr, "Quick-DRA: error: could not write to {}\n",
			           opts.path);
			return 1;
		}

		return 0;
	}
}  // namespace quick_dra::builtin::insured::remove
