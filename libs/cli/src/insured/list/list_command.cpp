// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <args_parser.hpp>
#include <filesystem>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/base/str.hpp>
#include <quick_dra/conv/low_level.hpp>
#include <quick_dra/models/types.hpp>

namespace quick_dra::builtin::insured::list {
	int handle(std::string_view tool_name,
	           args::arglist arguments,
	           std::string_view description) {
		std::optional<std::string> config_path;
		bool pipe{false};
		bool zero_pipe{false};

		args::null_translator tr{};
		args::parser parser{as_str(description), {tool_name, arguments}, &tr};

		parser.arg(config_path, "config")
		    .meta("<path>")
		    .help("select config file; defaults to ~/.quick_dra.yaml");

		parser.set<std::true_type>(pipe, "pipe")
		    .help("generate tab-separated output")
		    .opt();
		parser.set<std::true_type>(zero_pipe, "z")
		    .help("use zero as field separator, when --pipe is also used")
		    .opt();

		parser.parse();

		auto const path = platform::get_config_path(config_path);
		auto cfg = partial::config::load_partial(path, false);
		if (!cfg.insured) {
			cfg.insured.emplace();
		}

		if (pipe) {
			unsigned index = 0;
			for (auto const& person : *cfg.insured) {
				++index;
				std::string items[] = {
				    fmt::to_string(index),
				    person.last_name.value_or(""s),
				    person.first_name.value_or(""s),
				    person.kind.value_or(""s),
				    person.document.value_or(""s),
				    person.title
				        .transform(
				            [](auto const& value) { return as_string(value); })
				        .value_or(""s),
				    person.part_time_scale
				        .transform(
				            [](auto const& value) { return as_string(value); })
				        .value_or(""s),
				    person.salary
				        .transform(
				            [](auto const& value) { return as_string(value); })
				        .value_or(""s),
				};
				fmt::print("{}\n",
				           fmt::join(items, zero_pipe ? "\0"sv : "\t"sv));
			}

			return 0;
		}

		unsigned index = 0;
		for (auto const& person : *cfg.insured) {
			++index;
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

			fmt::print("#{}: {} {} [{} {}] {}\n", index,
			           person.first_name.value_or("??"),
			           person.last_name.value_or("??"),
			           person.kind.value_or("??"),
			           person.document.value_or("??"), part_time_salary);
		}

		return 0;
	}
}  // namespace quick_dra::builtin::insured::list
