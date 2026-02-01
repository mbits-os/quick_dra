// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include "cmd_conversation.hpp"
#include <quick_dra/base/paths.hpp>
#include <string>
#include <utility>

namespace quick_dra::builtin::payer {
	void cmd_conversation::parse_args(std::string_view tool_name,
	                                  args::arglist arguments,
	                                  std::string_view description) {
		std::optional<std::string> config_path;

		args::null_translator tr{};
		args::parser parser{as_str(description), {tool_name, arguments}, &tr};

		parser.arg(config_path, "file")
		    .meta("<path>")
		    .help("select config file; defaults to ~/.quick_dra.yaml");
		parser.set<std::false_type>(ask_questions, "y")
		    .help(
		        "use answers from command line and previous entries in "
		        "config "
		        "file; do not ask additional questions")
		    .opt();
		parser.arg(opts.first_name, "first")
		    .meta("<name>")
		    .help("select first name of the payer as answer");
		parser.arg(opts.last_name, "last")
		    .meta("<name>")
		    .help("select last name of the payer as answer");
		parser.arg(opts.social_id, "social-id")
		    .meta("<number>")
		    .help("select PESEL number as answer");
		parser.arg(opts.tax_id, "tax-id")
		    .meta("<number>")
		    .help("select NIP number as answer");
		parser.arg(opts.id_card, "id-card")
		    .meta("<number>")
		    .help(
		        "select state-issued id number as answer; if passport "
		        "is used "
		        "in the config file, it will be replaced by this "
		        "field");
		parser.arg(opts.passport, "passport")
		    .meta("<number>")
		    .help(
		        "select passport number as answer; if stated-issued id "
		        "is used "
		        "in the config file, it will be replaced by this "
		        "field");

		parser.parse();

#define RESET_EMPTY(N)               \
	if (opts.N && opts.N->empty()) { \
		opts.N.reset();              \
	}

		RESET_EMPTY(first_name);
		RESET_EMPTY(last_name);
		RESET_EMPTY(social_id);
		RESET_EMPTY(tax_id);
		RESET_EMPTY(id_card);
		RESET_EMPTY(passport);

#undef RESET_EMPTY

		if (opts.id_card && opts.passport) {
			parser.error(
			    "only one of --id-card and --passport can be used at the same "
			    "time");
		}

		path = platform::get_config_path(config_path);
	}
}  // namespace quick_dra::builtin::payer
