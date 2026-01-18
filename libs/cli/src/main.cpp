// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <args/parser.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/base/verbose.hpp>
#include <quick_dra/docs/file_set.hpp>
#include <quick_dra/docs/forms.hpp>
#include <quick_dra/docs/xml.hpp>
#include <quick_dra/docs/xml_builder.hpp>
#include <quick_dra/models/types.hpp>
#include <quick_dra/version.hpp>
#include "cli_options.hpp"

using namespace std::literals;

int main(int argc, char* argv[]) {
	using namespace quick_dra;

	auto const opt = options_from_cli(argc, argv);

	if (opt.verbose_level > verbose::none) {
		fmt::print("-- config used: {}\n", opt.config_path.string());
	}

	if (opt.verbose_level >= verbose::names_and_summary) {
		fmt::print("-- today: {}-{:02}-{:02}\n",
		           static_cast<int>(opt.today.year()),
		           static_cast<unsigned>(opt.today.month()),
		           static_cast<unsigned>(opt.today.day()));
	}

	fmt::print("-- report: #{} {}-{:02}\n", opt.report_index,
	           static_cast<int>(opt.date.year()),
	           static_cast<unsigned>(opt.date.month()));

	auto cfg = parse_config(opt.verbose_level, opt.date, opt.config_path);
	if (!cfg) {
		return 1;
	}

	auto raw_templates = quick_dra::templates::parse_yaml(
	    platform::config_data_dir() / "templates.yaml"sv);
	if (!raw_templates) {
		return 1;
	}

	auto const file =
	    build_file_set(opt, *cfg, compiled_templates::compile(*raw_templates));
	store_xml(file, set_filename(opt.report_index, opt.date), opt.indent_xml);

	if (opt.verbose_level >= verbose::last) {
		fmt::print("-- (no more info to unveil)\n");
	}
}
