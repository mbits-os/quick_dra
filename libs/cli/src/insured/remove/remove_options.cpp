// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "remove_options.hpp"
#include <optional>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/base/str.hpp>
#include <string>
#include <utility>
#include <vector>

namespace quick_dra::builtin::insured::remove {
	namespace {
		void add_search_from_position(args::parser& parser,
		                              options& out,
		                              unsigned position) {
			if (position < 1 || position > out.cfg.insured->size()) {
				if (out.cfg.insured->size() == 1) {
					parser.error("argument --pos must be equal to 1");
				} else {
					parser.error(
					    fmt::format("argument --pos must be between 1 and "
					                "{}, inclusive",
					                out.cfg.insured->size()));
				}
			}

			out.found.push_back(position - 1);
		}

		void add_search_from_records(
		    std::vector<unsigned>& found,
		    std::vector<std::vector<std::string>> const& records,
		    auto const& predicate) {
			unsigned index = 0;

			for (auto const& record : records) {
				auto matching = false;
				for (auto const& field : record) {
					if (predicate(field)) {
						matching = true;
						break;
					}
				}

				if (matching) {
					found.push_back(index);
				}

				++index;
			}
		}

		void add_search_from_keyword(args::parser& parser,
		                             options& out,
		                             std::string_view search_keyword) {
			std::vector<std::vector<std::string>> records{};
			records.reserve(out.cfg.insured->size());
			for (auto const& person : *out.cfg.insured) {
				records.emplace_back();
				auto& dst = records.back();
				dst.reserve(3);
				for (auto const& field :
				     {person.document, person.first_name, person.last_name}) {
					auto const key = field.transform(
					    [](auto const& fld) { return to_upper(fld); });
					if (key) {
						dst.push_back(std::move(*key));
					}
				}
			}

			auto const upper = to_upper(search_keyword);
			auto const view = std::string_view{upper};
			add_search_from_records(
			    out.found, records,
			    [view](std::string const& field) { return field == view; });

			if (out.found.empty()) {
				add_search_from_records(
				    out.found, records, [view](std::string const& field) {
					    return field.starts_with(view) || field.ends_with(view);
				    });
			}

			if (out.found.empty()) {
				parser.error(
				    fmt::format("--find: could not find any record using `{}'",
				                search_keyword));
			}
		}
	}  // namespace

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
		if (position) {
			add_search_from_position(parser, out, *position);
		} else {
			add_search_from_keyword(parser, out, *search_keyword);
		}

		return 0;
	}
}  // namespace quick_dra::builtin::insured::remove
