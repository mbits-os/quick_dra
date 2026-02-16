// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "cli_options.hpp"
#include <args_parser.hpp>
#include <map>
#include <quick_dra/base/paths.hpp>
#include <string>
#include "ctre_parsing.hpp"

namespace quick_dra::builtin::xml {
	namespace {
		template <typename D1, typename D2, typename Clock>
		time_point<Clock, D1> floor(time_point<Clock, D1> const& from) {
			auto const orig_dur = from.time_since_epoch();
			auto const casted_dur = duration_cast<D1>(orig_dur);
			return time_point<Clock, D1>{casted_dur};
		}

		year_month_day get_today() {
			auto const now = system_clock::now();
			auto const local =
			    floor<days>(zoned_time{current_zone(), now}.get_local_time());
			return year_month_day{local};
		}
	}  // namespace

	options options_from_cli(args::args_view const& arguments,
	                         std::string_view description) {
		std::optional<std::string> config_path;
		std::optional<std::filesystem::path> tax_config_path;
		unsigned verbose_counter{};
		int rel_month{-1};
		std::optional<std::string> today_str;
		unsigned report_index{1};
		bool indent_xml{false};
		bool print_info{false};

		args::null_translator tr{};
		args::parser parser{as_str(description), arguments, &tr};

		parser.custom([&] { ++verbose_counter; }, "v")
		    .help(
		        "set the output to be more verbose, "
		        "output will change with each added -v, e.g. -vv will differ "
		        "from -vvv")
		    .multi()
		    .opt();
		parser.arg(config_path, "config")
		    .meta("<path>")
		    .help("select config file; defaults to ~/.quick_dra.yaml");
		parser.arg(tax_config_path, "tax-config")
		    .meta("<path>")
		    .help(
		        "select tax parameters file; will take precedent before data "
		        "from repository and installation");
		parser.arg(report_index, "n")
		    .meta("<NN>")
		    .help(
		        "choose serial number of this particular report set; "
		        "defaults to 1")
		    .opt();
		parser.arg(rel_month, "m")
		    .meta("<month>")
		    .help(
		        "choose how many month away from today the report should use; "
		        "defaults to -1")
		    .opt();
		parser.arg(today_str, "today")
		    .meta("<YYYY-MM-DD>")
		    .help(
		        "choose the date for the XML production; defaults to date "
		        "setup on the host machine")
		    .opt();
		parser.set<std::true_type>(indent_xml, "pretty")
		    .help("pretty-print resulting XML document")
		    .opt();
		parser.set<std::true_type>(print_info, "info")
		    .help("end terminal printout with a summary of amounts to pay")
		    .opt();
		parser.parse();

		if (report_index < 1 || report_index > 99) {
			parser.error("serial number must be in range 1 to 99 inclusive");
		}

		auto const today_from_args = parse_date(today_str);
		if (!today_from_args && today_str) {
			parser.error(fmt::format("--today: expected YYYY-MM-DD, got `{}'",
			                         *today_str));
		}

		auto const today = today_from_args.value_or(get_today());
		auto const date =
		    year_month{today.year(), today.month()} + months{rel_month};
		return {.config_path = platform::get_config_path(config_path),
		        .tax_config_path = tax_config_path,
		        .verbose_level = verbose{verbose_counter},
		        .today = today,
		        .report_index = report_index,
		        .date = date,
		        .indent_xml = indent_xml,
		        .print_info = print_info};
	}  // GCOV_EXCL_LINE[WIN32]
}  // namespace quick_dra::builtin::xml
