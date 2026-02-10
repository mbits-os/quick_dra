// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <fmt/std.h>
#include <args_parser.hpp>
#include <quick_dra/base/str.hpp>
#include <quick_dra/conv/validators.hpp>
#include "add_conversation.hpp"

namespace quick_dra::builtin::insured::add {
	bool has_another_insured(partial::insured_t const& dst,
	                         std::vector<partial::insured_t> const& insured) {
		auto it =
		    std::find_if(insured.begin(), insured.end(), [&](auto const& item) {
			    return item.kind == dst.kind && item.document == dst.document;
		    });
		if (it == insured.end()) {
			return false;
		}

		auto const index = (it - insured.begin()) + 1;
		auto const& person = *it;
		fmt::print(
		    "Found another person with this document:\n"
		    "\n"
		    "    #{0}: {1} {2} [{3} {4}]\n"
		    "\n"
		    "Either remove this person or call\n"
		    "\n"
		    "    qdra insured edit --pos {0}\n"
		    "\n",
		    index, person.first_name.value_or("??"),
		    person.last_name.value_or("??"), person.kind.value_or("??"),
		    person.document.value_or("??"));

		return true;
	}

	int handle(std::string_view tool_name,
	           args::arglist arguments,
	           std::string_view description) {
		conversation conv{};
		conv.parse_args(tool_name, arguments, description);

		auto cfg = partial::config::load_partial(conv.path);
		if (!cfg.insured) {
			cfg.insured.emplace();
		}

		if (!conv.check_field(policies::first_name) ||
		    !conv.check_field(policies::last_name) ||
		    !conv.check_enum_field(""sv, policies::kind, policies::document,
		                           get_enum_item(policies::social_id),
		                           get_enum_item(policies::id_card),
		                           get_enum_item(policies::passport)) ||
		    has_another_insured(conv.dst, *cfg.insured) ||
		    !conv.check_field(policies::title) ||
		    !conv.check_field(policies::part_time_scale) ||
		    !conv.check_field(policies::salary)) {
			return 1;
		}

		conv.show_added(policies::first_name);
		conv.show_added(policies::last_name);
		conv.show_added(policies::kind);
		conv.show_added(policies::document);
		conv.show_added(policies::title);
		conv.show_added(policies::part_time_scale);
		conv.show_added(policies::salary);

		if (conv.dst.part_time_scale == full_time) {
			conv.dst.part_time_scale.reset();
		}

		if (conv.dst.salary == minimal_salary) {
			// minimal, in config denoted by undefined value
			conv.dst.salary.reset();
		}

		cfg.insured->push_back(std::move(conv.dst));
		if (!cfg.store(conv.path)) {
			fmt::print(stderr, "Quick-DRA: error: could not write to {}\n",
			           conv.path);
			return 1;
		}
		return 0;
	}
}  // namespace quick_dra::builtin::insured::add
