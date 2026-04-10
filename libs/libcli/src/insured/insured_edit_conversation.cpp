// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "insured_edit_conversation.hpp"
#include <optional>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/base/types.hpp>
#include <quick_dra/conv/validators.hpp>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace quick_dra::builtin::insured::edit {
	conversation::conversation(std::string_view tool_name,
	                           args::arglist arguments,
	                           std::string_view description)
	    : arg_parser{tool_name, arguments, description} {}

	void conversation::parse_args() {
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
		parser.set<std::false_type>(ask_questions, "y")
		    .help(
		        "use answers from command line; do not ask additional "
		        "questions")
		    .opt();
		parser.arg(opts.first_name, "first")
		    .meta("<name>")
		    .help("provide first name of the payer");
		parser.arg(opts.last_name, "last")
		    .meta("<name>")
		    .help("provide last name of the payer");
		parser.arg(opts.social_id, "social-id")
		    .meta("<number>")
		    .help("provide PESEL number");
		parser.arg(opts.id_card, "id-card")
		    .meta("<number>")
		    .help("provide state-issued id number");
		parser.arg(opts.passport, "passport")
		    .meta("<number>")
		    .help("provide passport number");
		parser.arg(opts.title, "title")
		    .meta("<code>")
		    .help(
		        "provide insurance title code as six digits in `#### # #' "
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
		        "provide gross salary amount, before applying the scale, "
		        "represented by a number with 0.01 increment, with optional "
		        "PLN or zł suffix; alternatively, single word \"minimal\" to "
		        "represent a minimal pay in a given month");

		parser.parse();

		if (!position && !search_keyword) {
			parser.error("one of --pos and --find argument is required");
		}

		if (position && search_keyword) {
			parser.error("only one of --pos and --find is allowed");
		}

		if (position) {
			search_term = *position;
		} else {
			search_term = std::move(*search_keyword);
		}

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
			opts.salary = minimal_salary;
		}

#undef RESET_EMPTY

		path = platform::get_config_path(config_path);
	}

	void conversation::check_required() {
		verifier(parser)
		    .required(policies::first_name.through("--first"sv))
		    .required(policies::last_name.through("--last"sv))
		    .required(policies::social_id.through("--social-id"sv),
		              policies::id_card.through("--id-card"sv),
		              policies::passport.through("--passport"sv))
		    .required(policies::title.through("--title"sv));
	}
}  // namespace quick_dra::builtin::insured::edit
