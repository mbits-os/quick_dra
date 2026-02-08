// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "builtins.hpp"
#include <fmt/format.h>
#include <quick_dra/version.hpp>

namespace quick_dra::builtin {
	namespace {
		[[noreturn]] void show_version() {
			fmt::print("{} version {}\n", version::program, version::ui);
			std::exit(0);
		}
	}  // namespace

	void help_group::copy_to(args::chunk& chunk) const {
		chunk.title.assign(name);
		chunk.items.reserve(commands.size());
		for (auto [cmd_name, description] : commands)
			chunk.items.push_back({as_str(cmd_name), as_str(description)});
	}

	void help_group::add_to_parser(args::parser& parser,
	                               std::span<help_group const> const& groups) {
		parser.provide_help(false);
		parser
		    .custom(
		        [command_groups = groups] [[noreturn]] (args::parser & p) {
			        help_group::show_help(p, command_groups);
		        },
		        "h", "help")
		    .help("shows this help message and exits"sv)
		    .opt();
	}

	void help_group::fill_help(args::fmt_list& commands,
	                           std::span<help_group const> const& groups) {
		commands.reserve(commands.size() + groups.size());
		for (auto const& group : groups) {
			commands.emplace_back();
			group.copy_to(commands.back());
		}
	}

	[[noreturn]] void help_group::show_help(
	    args::parser& parser,
	    std::span<help_group const> const& groups) {
		auto commands = parser.printer_arguments();
		fill_help(commands, groups);

		parser.short_help();
		args::printer{stdout}.format_list(commands);
		std::exit(0);
	}

	parser::parser(std::string_view description,
	               args::args_view const& args,
	               std::span<help_group const> const& groups,
	               bool is_root)
	    : parser_{as_str(description), args, &tr_}, groups_{groups} {
		if (is_root) {
			parser_.usage("[-h] [--version] <command> [<args>]"sv);
		} else {
			parser_.usage("[-h] <command> [<args>]"sv);
		}
		builtin::help_group::add_to_parser(parser_, groups);
	}

	root_parser::root_parser(args::args_view const& args,
	                         std::span<help_group const> const& groups)
	    : parser{""sv, args, groups, true} {
		get_parser()
		    .custom(show_version, "v", "version")
		    .help("show version and exit")
		    .opt();
	}

	options parser::parse_args() {
		auto const unparsed = parser_.parse(args::parser::allow_subcommands);
		if (unparsed.empty()) {
			builtin::help_group::show_help(parser_, groups_);
		}

		auto const tool = unparsed[0];
		if (!tool.empty() && tool.front() == '-')
			parser_.error(parser_.tr()(args::lng::unrecognized, tool));

		return {tool, unparsed.shift()};
	}

	void parser::noent(std::string_view tool, std::string_view my_name) const {
		args::fmt_list commands{};
		builtin::help_group::fill_help(commands, groups_);
		parser_.short_help(stderr);

		auto prn = args::printer{stderr};
		auto msg = fmt::format("\"{}\" is not a {} command", tool, my_name);
		prn.format_paragraph(fmt::format("{}: {}", parser_.program(), msg), 0);
		prn.format_list(commands);
	}
}  // namespace quick_dra::builtin
