// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <fmt/std.h>
#include <args_parser.hpp>
#include <quick_dra/conv/low_level.hpp>
#include "cmd_conversation.hpp"
#include "validators.hpp"

namespace quick_dra::builtin::payer {
	int handle(std::string_view tool_name,
	           args::arglist arguments,
	           std::string_view description) {
		cmd_conversation conv{};
		conv.parse_args(tool_name, arguments, description);

		auto cfg = partial::config::load_partial(conv.path);

		if (!cfg.payer) {
			cfg.payer.emplace();
		}

		conv.dst = *cfg.payer;

		if (!conv.check_string_field(policies::first_name) ||
		    !conv.check_string_field(policies::last_name) ||
		    !conv.check_string_field(policies::tax_id) ||
		    !conv.check_string_field(policies::social_id)) {
			return 1;
		}

		std::string kind;
		if (conv.ask_questions) {
			if (!get_enum_answer(
			        "Document type"sv,
			        std::array{
			            std::pair{'1', "ID card"sv},
			            std::pair{'2', "Passport"sv},
			        },
			        [&](char code) { kind = std::string{code}; })) {
				return 1;
			}
		} else {
			if (conv.opts.id_card) {
				kind = "1"s;
			} else if (conv.opts.passport) {
				kind = "2"s;
			} else if (conv.dst.kind) {
				kind = *conv.dst.kind;
			} else {
				comment(
				    "Cannot guess document kind based on information given. "
				    "Please use either --id-card or --passport with -y\n");
				return 1;
			}
		}

		conv.dst.preprocess_document();
		if (kind == "1"sv) {
			if (!conv.check_string_field(policies::id_card)) return 1;
			conv.dst.passport.reset();
		} else {
			if (!conv.check_string_field(policies::passport)) return 1;
			conv.dst.id_card.reset();
		}
		conv.dst.postprocess_document();

		conv.show_modified(policies::first_name, *cfg.payer);
		conv.show_modified(policies::last_name, *cfg.payer);
		conv.show_modified(policies::tax_id, *cfg.payer);
		conv.show_modified(policies::social_id, *cfg.payer);
		conv.show_modified(policies::kind, *cfg.payer);
		conv.show_modified(policies::document, *cfg.payer);

		cfg.payer = std::move(conv.dst);
		if (!cfg.store(conv.path)) {
			fmt::print(stderr, "Quick-DRA: error: could not write to {}\n",
			           conv.path);
			return 1;
		}
		return 0;
	}
}  // namespace quick_dra::builtin::payer
