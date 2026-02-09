// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "payer_conversation.hpp"
#include <args_parser.hpp>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/conv/validators.hpp>
#include <string>
#include <string_view>
#include <type_traits>

namespace quick_dra::builtin::payer {
	conversation::conversation(std::string_view tool_name,
	                           args::arglist arguments,
	                           std::string_view description)
	    : arg_parser{tool_name, arguments, description} {}

	void conversation::parse_args() {
		std::optional<std::string> config_path;

		parser.arg(config_path, "config")
		    .meta("<path>")
		    .help("select config file; defaults to ~/.quick_dra.yaml");
		parser.set<std::false_type>(ask_questions, "y")
		    .help(
		        "use answers from command line and previous entries in config "
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
		        "select state-issued id number as answer; if passport is used "
		        "in the config file, it will be replaced by this field");
		parser.arg(opts.passport, "passport")
		    .meta("<number>")
		    .help(
		        "select passport number as answer; if stated-issued id is used "
		        "in the config file, it will be replaced by this field");

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

		path = platform::get_config_path(config_path);
	}

	void conversation::check_required() {
		verifier(parser)
		    .required(policies::first_name.through("--first"sv))
		    .required(policies::last_name.through("--last"sv))
		    .required(policies::social_id.through("--social-id"sv))
		    .required(policies::tax_id.through("--tax-id"sv))
		    .required(policies::id_card.through("--id-card"sv),
		              policies::passport.through("--passport"sv));
	}
}  // namespace quick_dra::builtin::payer
