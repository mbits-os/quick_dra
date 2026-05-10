// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/std.h>
#include <quick_dra/base/str.hpp>
#include <quick_dra/conv/args_parser.hpp>
#include <quick_dra/conv/search.hpp>
#include <quick_dra/conv/validators.hpp>
#include <string>
#include <vector>
#include "insured_edit_conversation.hpp"

namespace quick_dra::builtin::insured::edit {
	bool wrap_document_edit(conversation& conv, unsigned pos, std::vector<partial::insured_t> const& insured) {
		while (true) {
			if (!conv.check_enum_field("--social-id, --id-card or --passport"sv, policies::kind, policies::document,
			                           get_enum_item(policies::social_id), get_enum_item(policies::id_card),
			                           get_enum_item(policies::passport))) {
				return false;
			}

			unsigned index = 0;
			for (auto const& person : insured) {
				if (index == pos) {
					++index;
					continue;
				}

				if (person.kind == conv.dst.kind && person.document == conv.dst.document) {
					break;
				}

				++index;
			}

			if (index == insured.size()) {
				return true;
			}

			auto const& person = insured[index];
			fmt::print(
			    "Found another person with this document:\n"
			    "\n"
			    "    #{}: {} {} [{} {}]\n"
			    "\n",
			    index, person.first_name.value_or("??"), person.last_name.value_or("??"), person.kind.value_or("??"),
			    person.document.value_or("??"));
		}
	}

	int handle(std::string_view tool_name, args::arglist arguments, std::string_view description) {
		conversation conv{tool_name, arguments, description};
		conv.parse_args();

		auto cfg = partial::config::load_partial(conv.path);

		if (!cfg.insured) {
			cfg.insured.emplace();
		}

		auto found = search_insured_from_term(conv.search_term, *cfg.insured, match_level::none, nullptr,
		                                      [&conv](std::string const& msg) { conv.parser.error(msg); });

		year_month on_month{};
		partial::employment_history employment{};
		auto had_this_date = false;

		if (found.size() == 1) {
			std::tie(on_month, employment) = *conv.opts.history->rbegin();
			auto& orig = cfg.insured->at(found.front());
			if (!orig.history) {
				orig.history.emplace();
			}

			if (on_month == null_month) {
				on_month = last_date_or_today(on_month, *orig.history);
			}

			auto it = conv.opts.history->rbegin();
			auto opts = it->second;
			conv.opts.history->erase(it->first);
			(*conv.opts.history)[on_month] = opts;

			had_this_date = orig.history->count(on_month) > 0;

			if (!policies::part_time_scale[on_month].select(orig)) {
				policies::part_time_scale[on_month].select(orig) = full_time;
			}
			if (!policies::salary[on_month].select(orig)) {
				policies::salary[on_month].select(orig) = minimal_salary;
			}

			conv.dst = orig;
			conv.check_required();
		}

		if (!found.empty()) {
			fmt::print("Found:\n");
		}

		for (auto const index : found) {
			auto const& person = cfg.insured->at(index);
			fmt::print("    #{0}: {1} {2} [{3} {4}]\n", index + 1, person.first_name.value_or("??"),
			           person.last_name.value_or("??"), person.kind.value_or("??"), person.document.value_or("??"));
		}

		if (found.size() > 1) {
			fmt::print(
			    "\n"
			    "This search was ambiguous. Please refine your parameter or "
			    "use --pos to pinpoint the record.\n");
			return 1;
		}

		auto const part_time_scale = policies::part_time_scale[on_month];
		auto const salary = policies::salary[on_month];

		if (!conv.check_field(policies::first_name) || !conv.check_field(policies::last_name) ||
		    !wrap_document_edit(conv, found.front(), *cfg.insured) || !conv.check_field(policies::title) ||
		    !conv.check_field(part_time_scale) || !conv.check_field(salary)) {
			return 1;
		}

		auto& orig = cfg.insured->at(found.front());

		conv.show_modified(policies::first_name, orig);
		conv.show_modified(policies::last_name, orig);
		conv.show_modified(policies::kind, orig);
		conv.show_modified(policies::document, orig);
		conv.show_modified(policies::title, orig);

		if (had_this_date) {
			conv.show_modified(part_time_scale, orig);
			conv.show_modified(salary, orig);
		} else {
			conv.show_added(part_time_scale);
			conv.show_added(salary);
		}

		if (part_time_scale.select(conv.dst) == full_time) {
			part_time_scale.select(conv.dst).reset();
		}

		if (salary.select(conv.dst) == minimal_salary) {
			salary.select(conv.dst).reset();
		}

		orig = conv.dst;
		if (!cfg.store(conv.path)) {
			fmt::print(stderr, "Quick-DRA: error: could not write to {}\n", conv.path);
			return 1;
		}
		return 0;
	}
}  // namespace quick_dra::builtin::insured::edit
