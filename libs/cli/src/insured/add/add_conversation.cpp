// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "add_conversation.hpp"
#include <optional>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/base/types.hpp>
#include <quick_dra/conv/validators.hpp>
#include <string>
#include <string_view>
#include <type_traits>

namespace quick_dra::builtin::insured::add {
	void conversation::parse_args(std::string_view tool_name,
	                              args::arglist arguments,
	                              std::string_view description) {
		std::optional<std::string> config_path;

		args::null_translator tr{};
		args::parser parser{as_str(description), {tool_name, arguments}, &tr};

		parser.arg(config_path, "config")
		    .meta("<path>")
		    .help("select config file; defaults to ~/.quick_dra.yaml");
		parser.set<std::false_type>(ask_questions, "y")
		    .help(
		        "use answers from command line; do not ask additional "
		        "questions")
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
		parser.arg(opts.id_card, "id-card")
		    .meta("<number>")
		    .help("select state-issued id number as answer");
		parser.arg(opts.passport, "passport")
		    .meta("<number>")
		    .help("select passport number as answer");
		parser.arg(opts.title, "title")
		    .meta("<code>")
		    .help(
		        "select insurance title code as six digits in `#### # #' "
		        "format; for instance, for title of 0110, no social benefits, "
		        "no disability, it should be \"0110 0 0\"");
		parser.arg(opts.part_time_scale, "scale")
		    .meta("<num>/<den>")
		    .help(
		        "for part time workers, what scale should be applied to their "
		        "salary; defaults to 1/1");
		parser.arg(opts.salary, "salary")
		    .meta("<zł>")
		    .help(
		        "select gross salary amount, before applying the scale, "
		        "represented by a number with 0.01 increment, with optional "
		        "PLN or zł suffix; alternatively, single word \"minimal\" to "
		        "represent a minimal pay in a given month");

		parser.parse();

#define RESET_EMPTY(N)     \
	if (N && N->empty()) { \
		N.reset();         \
	}

		RESET_EMPTY(opts.first_name);
		RESET_EMPTY(opts.last_name);
		RESET_EMPTY(opts.social_id);
		RESET_EMPTY(opts.id_card);
		RESET_EMPTY(opts.passport);

		if (!opts.salary) {
			opts.salary = currency{-1 * currency::den};
		}

#undef RESET_EMPTY

		verifier(parser)
		    .required(policies::first_name.through("--first"sv))
		    .required(policies::last_name.through("--last"sv))
		    .required(policies::social_id.through("--social-id"sv),
		              policies::id_card.through("--id-card"sv),
		              policies::passport.through("--passport"sv))
		    .required(policies::title.through("--title"sv));

		if (opts.id_card && opts.passport) {
			parser.error(
			    "only one of --id-card and --passport can be used at the same "
			    "time");
		} else if (opts.id_card && opts.social_id) {
			parser.error(
			    "only one of --id-card and --social-id can be used at the same "
			    "time");
		} else if (opts.social_id && opts.passport) {
			parser.error(
			    "only one of --social-id and --passport can be used at the "
			    "same time");
		}

		path = platform::get_config_path(config_path);
	}
}  // namespace quick_dra::builtin::insured::add
