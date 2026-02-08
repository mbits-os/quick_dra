// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <fmt/std.h>
#include <args_parser.hpp>
#include <quick_dra/conv/low_level.hpp>
#include <quick_dra/conv/validators.hpp>
#include "payer_conversation.hpp"

namespace quick_dra::builtin::payer {
	int handle(std::string_view tool_name,
	           args::arglist arguments,
	           std::string_view description) {
		conversation conv{tool_name, arguments, description};
		conv.parse_args();

		auto cfg = partial::config::load_partial(conv.path);

		if (!cfg.payer) {
			cfg.payer.emplace();
		}

		conv.dst = *cfg.payer;
		conv.check_required();

		if (!conv.check_field(policies::first_name) ||
		    !conv.check_field(policies::last_name) ||
		    !conv.check_field(policies::tax_id) ||
		    !conv.check_field(policies::social_id) ||
		    !conv.check_enum_field("--id-card or --passport"sv, policies::kind,
		                           get_enum_item(policies::id_card),
		                           get_enum_item(policies::passport))) {
			return 1;
		}

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
