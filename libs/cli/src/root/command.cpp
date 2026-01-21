// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <args/parser.hpp>
#include <quick_dra/base/str.hpp>
#include <quick_dra/version.hpp>
#include <span>
#include "../builtins.hpp"

namespace quick_dra::root {
	namespace {
		struct help_command {
			std::string_view name;
			std::string_view description;
		};

		struct help_group {
			std::string_view name;
			std::span<help_command const> commands;
		};

#define BUILTINS_X_KNOWN(NAME, TOOL, DSCR) {TOOL##sv, DSCR##sv},
		static constexpr help_command builtin_commands[] = {
		    BUILTINS_X(BUILTINS_X_KNOWN)};
#undef BUILTINS_X_KNOWN

		static constexpr help_group known_commands[] = {
		    {"known commands"sv, builtin_commands},
		};

		void copy_commands(args::chunk& chunk, help_group const& group) {
			chunk.title.assign(group.name);
			chunk.items.reserve(group.commands.size());
			for (auto [name, description] : group.commands)
				chunk.items.push_back({as_str(name), as_str(description)});
		}

		[[noreturn]] void show_help(args::parser& p) {
			auto commands = p.printer_arguments();
			commands.reserve(commands.size() + std::size(known_commands));
			for (auto const& group : known_commands) {
				commands.emplace_back();
				copy_commands(commands.back(), group);
			}

			p.short_help();
			args::printer{stdout}.format_list(commands);
			std::exit(0);
		}
	}  // namespace

	struct options {
		std::string_view tool_name{};
		args::arglist args{};
	};

	class parser {
	public:
		parser(args::args_view const& args);
		options parse_args();
		void noent(std::string_view tool) const;

	private:
		args::null_translator tr_{};
		args::parser parser_;
	};

	parser::parser(args::args_view const& args) : parser_{"", args, &tr_} {
		parser_.usage("[-h] [--version] <command> [<args>]"sv);
		parser_.provide_help(false);
		parser_.custom(show_help, "h", "help")
		    .help("shows this help message and exits"sv)
		    .opt();
		parser_
		    .custom(
		        [] {
			        fmt::print("{} version {}\n", version::program,
			                   version::ui);
			        std::exit(0);
		        },
		        "v", "version")
		    .help("show version and exit")
		    .opt();
	}
	options parser::parse_args() {
		auto const unparsed = parser_.parse(args::parser::allow_subcommands);
		if (unparsed.empty()) show_help(parser_);

		auto const tool = unparsed[0];
		if (!tool.empty() && tool.front() == '-')
			parser_.error(parser_.tr()(args::lng::unrecognized, tool));

		return {tool, unparsed.shift()};
	}

	void parser::noent(std::string_view tool) const {
		args::fmt_list commands{};
		commands.reserve(std::size(known_commands));
		for (auto const& group : known_commands) {
			commands.emplace_back();
			copy_commands(commands.back(), group);
		}

		parser_.short_help(stderr);

		auto prn = args::printer{stderr};
		auto msg = fmt::format("\"{}\" is not a qdra command", tool);
		prn.format_paragraph(fmt::format("{}: {}", parser_.program(), msg), 0);
		prn.format_list(commands);
	}
}  // namespace quick_dra::root

int tool(args::args_view const& arguments) {
	using namespace quick_dra;

	root::parser parser{arguments};
	auto const [tool, tool_args] = parser.parse_args();

	auto const ret = tools{builtin::tools}.handle(tool, tool_args);
	if (ret == -ENOENT) {
		parser.noent(tool);
		return 1;
	}

	if (ret < 0) return -ret;
	return ret;
}
