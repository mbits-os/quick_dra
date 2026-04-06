// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <array>
#include <filesystem>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/base/str.hpp>
#include <quick_dra/conv/args_parser.hpp>
#include <quick_dra/conv/low_level.hpp>
#include <quick_dra/conv/search.hpp>
#include <quick_dra/models/types.hpp>
#include <string>

namespace quick_dra::builtin::list {
	int handle(std::string_view tool_name,
	           args::arglist arguments,
	           std::string_view description) {
		std::optional<std::string> config_path;
		std::optional<std::string> search_keyword;
		bool pipe{false};
		bool zero_pipe{false};

		args::null_translator tr{};
		args::parser parser{as_str(description), {tool_name, arguments}, &tr};

		parser.arg(config_path, "config")
		    .meta("<path>")
		    .help("select config file; defaults to ~/.quick_dra.yaml");

		parser.arg(search_keyword, "find")
		    .meta("<keyword>")
		    .help(
		        "first or last name, or a document number to use as a "
		        "search key");

		parser.set<std::true_type>(pipe, "pipe")
		    .help("generate tab-separated output")
		    .opt();
		parser.set<std::true_type>(zero_pipe, "z")
		    .help("use zero as field separator, when --pipe is also used")
		    .opt();

		parser.parse();

		auto payer_matches = match_level::none;
		auto insured_matches = match_level::none;
		auto const path = platform::get_config_path(config_path);
		auto cfg = partial::config::load_partial(path, false);

		if (cfg.payer) {
			if (search_keyword) {
				payer_matches =
				    match_payer_from_keyword(*search_keyword, *cfg.payer);
			} else {
				payer_matches = match_level::direct;
			}
		}

		if (!cfg.insured) {
			cfg.insured.emplace();
		}

		std::vector<unsigned> found{};
		if (search_keyword) {
			found = search_insured_from_keyword(
			    *search_keyword, *cfg.insured, payer_matches, &insured_matches,
			    [&parser](std::string const& msg) { parser.error(msg); });
		} else {
			auto const size = cfg.insured->size();
			found.reserve(size);
			for (size_t index = 0; index < size; ++index) {
				found.push_back(static_cast<unsigned>(index));
			}
			insured_matches = match_level::direct;
		}

		if (insured_matches == match_level::direct &&
		    payer_matches == match_level::partial) {
			payer_matches = match_level::none;
		}

		if (pipe) {
			if (payer_matches != match_level::none) {
				auto const& person = *cfg.payer;
				auto items = std::array{
				    "P"s,
				    person.last_name.value_or(""s),  // GCOV_EXCL_LINE[GCC]
				    person.first_name.value_or(""s),
				    person.kind.value_or(""s),
				    person.document.value_or(""s),
				    person.tax_id.value_or(""s),
				    person.social_id.value_or(""s),
				};
				fmt::print("{}\n",
				           fmt::join(items, zero_pipe ? "\0"sv : "\t"sv));
			}

			for (unsigned const index : found) {
				auto const& person = (*cfg.insured)[index];
				auto items = std::array{
				    fmt::to_string(index + 1),
				    person.last_name.value_or(""s),  // GCOV_EXCL_LINE[GCC]
				    person.first_name.value_or(""s),
				    person.kind.value_or(""s),
				    person.document.value_or(""s),
				    person.title
				        .transform(  // GCOV_EXCL_LINE[GCC]
				            [](auto const& value) { return as_string(value); })
				        .value_or(""s),
				    person.part_time_scale
				        .transform(  // GCOV_EXCL_LINE[GCC]
				            [](auto const& value) { return as_string(value); })
				        .value_or(""s),
				    person.salary
				        .transform(  // GCOV_EXCL_LINE[GCC]
				            [](auto const& value) { return as_string(value); })
				        .value_or(""s),
				};
				fmt::print("{}\n",
				           fmt::join(items, zero_pipe ? "\0"sv : "\t"sv));
			}

			return 0;
		}

		if (payer_matches != match_level::none) {
			auto const& person = *cfg.payer;

			fmt::print(
			    "Payer: {} {} [{} {}] PESEL:{} NIP:{}\n",
			    person.first_name.value_or("??"),
			    person.last_name.value_or("??"), person.kind.value_or("??"),
			    person.document.value_or("??"), person.social_id.value_or("??"),
			    person.tax_id.value_or("??"));
		}

		for (unsigned const index : found) {
			auto const& person = (*cfg.insured)[index];
			std::string part_time_salary =
			    person.salary
			        .transform(
			            [](auto const& value) { return as_string(value); })
			        .value_or("<minimal>"s);

			if (person.part_time_scale &&
			    *person.part_time_scale != full_time) {
				part_time_salary =
				    fmt::format("{} of {}", as_string(*person.part_time_scale),
				                part_time_salary);
			}

			fmt::print("#{}: {} {} [{} {}] {}\n", index + 1,
			           person.first_name.value_or("??"),
			           person.last_name.value_or("??"),
			           person.kind.value_or("??"),
			           person.document.value_or("??"), part_time_salary);
		}

		return 0;
	}
}  // namespace quick_dra::builtin::list
