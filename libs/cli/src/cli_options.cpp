// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "cli_options.hpp"
#include <args/parser.hpp>
#include <map>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/version.hpp>
#include <string>

namespace quick_dra {
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

		std::filesystem::path get_config_path(
		    std::optional<std::string> const& override) {
			if (override) return *override;

			return platform::home_path() / ".quick_dra.yaml"sv;
		}

		currency find_minimal(year_month const& key,
		                      std::map<year_month, currency> const& minimal) {
			year_month result_date{};
			currency result{};

			for (auto const& [date, amount] : minimal) {
				if (key < date) continue;
				if (result_date > date) continue;
				result_date = date;
				result = amount;
			}

			return result;
		}
	}  // namespace

	options options_from_cli(int argc, char* argv[]) {
		std::optional<std::string> config_path;
		unsigned verbose_counter{};
		int rel_month{-1};
		unsigned report_index{1};
		bool indent_xml{false};

		args::null_translator tr{};
		args::parser parser{"", args::from_main(argc, argv), &tr};

		parser
		    .custom(
		        [] {
			        fmt::print("{} version {}\n", version::program,
			                   version::ui);
			        std::exit(0);
		        },
		        "V", "version")
		    .help("show version and exit")
		    .opt();
		parser.custom([&] { ++verbose_counter; }, "v")
		    .help("sets the output to be more verbose")
		    .opt();
		parser.arg(config_path, "config")
		    .meta("<path>")
		    .help("selects file other, than ~/.quick_dra.yaml");
		parser.arg(report_index, "n")
		    .meta("<NN>")
		    .help("serial number of this particular report set; defaults to 1")
		    .opt();
		parser.arg(rel_month, "m")
		    .meta("<month>")
		    .help(
		        "how many month away from today to generate reports for; "
		        "defaults "
		        "to -1")
		    .opt();
		parser.set<std::true_type>(indent_xml, "pretty")
		    .help("pretty-prints resulting XML document")
		    .opt();
		parser.parse();

		if (report_index < 1 || report_index > 99) {
			parser.error(fmt::format(
			    "serial number must be in range 1 to 99 inclusive"));
		}

		auto const today = get_today();
		return {
		    .config_path = get_config_path(config_path),
		    .verbose_level = verbose{verbose_counter},
		    .today = today,
		    .report_index = report_index,
		    .date = year_month{today.year(), today.month()} + months{rel_month},
		    .indent_xml = indent_xml,
		};
	}
}  // namespace quick_dra
