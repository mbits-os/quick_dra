// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <args/parser.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <quick_dra/app/version.hpp>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/docs/forms.hpp>
#include <quick_dra/docs/xml.hpp>
#include <quick_dra/docs/xml_builder.hpp>
#include <quick_dra/model/types.hpp>

using namespace std::literals;

namespace quick_dra {
	std::filesystem::path get_config_path(
	    std::optional<std::string> const& override) {
		if (override) return *override;

		return platform::home_path() / ".quick_dra.yaml"sv;
	}

	std::optional<config> parse_config(bool debug,
	                                   std::filesystem::path const& path) {
		if (debug) {
			fmt::print("using config from: {}\n", path.string());
		}

		auto result = config::parse_yaml(path);
		if (!result) return result;

		auto minimal =
		    config::parse_minimal_only(platform::data_dir() / "config.yaml"sv);

		if (minimal) {
			result->minimal.merge(*minimal);
		}

		if (debug) {
			result->debug_print();
		}

		return result;
	}

	using namespace std::chrono;

	template <typename D1, typename D2, typename Clock>
	time_point<Clock, D1> floor(time_point<Clock, D1> const& from) {
		auto const orig_dur = from.time_since_epoch();
		auto const casted_dur = duration_cast<D1>(orig_dur);
		return time_point<Clock, D1>{casted_dur};
	}

	year_month_day today() {
		auto const now = system_clock::now();
		auto const local =
		    floor<days>(zoned_time{current_zone(), now}.get_local_time());
		return year_month_day{local};
	}

	year_month get_year_month(months rel_months) {
		auto const ymd = today();
		return year_month{ymd.year(), ymd.month()} + rel_months;
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

	std::string set_filename(unsigned report_index, year_month const& date) {
		return fmt::format("quick-dra_{}{:02}-{:02}.xml",
		                   static_cast<int>(date.year()),
		                   static_cast<unsigned>(date.month()), report_index);
	}
}  // namespace quick_dra

int main(int argc, char* argv[]) {
	using namespace quick_dra;

	std::optional<std::string> config_path;
	bool verbose{false};
	int rel_month{-1};
	unsigned report_index{1};

	args::null_translator tr{};
	args::parser parser{"", args::from_main(argc, argv), &tr};

	parser
	    .custom(
	        []() {
		        fmt::print("{} version {}\n", version::program, version::ui);
		        std::exit(0);
	        },
	        "v", "version")
	    .help("show version and exit")
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
	parser.set<std::true_type>(verbose, "V")
	    .help("sets the output to be more verbose")
	    .opt();
	parser.parse();

	if (report_index < 1 || report_index > 99) {
		parser.error(
		    fmt::format("serial number must be in range 1 to 99 inclusive"));
	}

	auto const today = quick_dra::today();
	auto const date =
	    year_month{today.year(), today.month()} + months{rel_month};

	fmt::print("report: {:02} {}-{:02}\n", report_index,
	           static_cast<int>(date.year()),
	           static_cast<unsigned>(date.month()));
	fmt::print("config: {}\n", get_config_path(config_path).string());
	auto cfg = parse_config(verbose, get_config_path(config_path));
	if (!cfg) {
		return 1;
	}

	auto raw_templates = quick_dra::templates::parse_yaml(platform::data_dir() /
	                                                      "templates.yaml"sv);
	if (!raw_templates) {
		return 1;
	}

	auto templates = compiled_templates::compile(*raw_templates);

	cfg->params.minimal_pay = find_minimal(date, cfg->minimal);

	if (verbose) {
		templates.debug_print();
		fmt::print("Today: {}-{:02}-{:02}\n", static_cast<int>(today.year()),
		           static_cast<unsigned>(today.month()),
		           static_cast<unsigned>(today.day()));
		fmt::print("Minimal pay: {:.02f} zł\n", cfg->params.minimal_pay);
	}

	auto doc_id = 0u;
	auto root = build_kedu_doc(version::program, version::string);

	for (auto const& form :
	     prepare_form_set(verbose, report_index, date, today, *cfg)) {
		auto it = templates.reports.find(form.key);
		if (it == templates.reports.end()) continue;
		attach_document(root, verbose, form, it->second, ++doc_id);
	}

	store_xml(root, set_filename(report_index, date));
}
