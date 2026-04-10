// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <quick_dra/base/str.hpp>
#include <quick_dra/cli/commands.hpp>
#include <quick_dra/conv/args_parser.hpp>
#include <span>
#include <string_view>

namespace quick_dra::builtin {
	using namespace std::literals;

#define ROOT_BUILTINS_X(X)                                                  \
	X(payer, "payer", "provide/modify the identity of the insurance payer") \
	X(insured, "insured", "manage the insured people data")                 \
	X(list, "list", "list people in configuration")                         \
	X(xml, "xml", "produce KEDU 5.6 XML file")

#define INSURED_BUILTINS_X(X)                                   \
	X(add, "add", "add a new insured person")                   \
	X(edit, "edit", "update data of a selected insured person") \
	X(remove, "remove", "remove an insured person from configuration")

#define BUILTINS_X_DECL(NAME, TOOL, DSCR)         \
	namespace NAME {                              \
		int handle(std::string_view tool,         \
		           args::arglist args,            \
		           std::string_view description); \
	}
	ROOT_BUILTINS_X(BUILTINS_X_DECL)

	namespace insured {
		INSURED_BUILTINS_X(BUILTINS_X_DECL)
	}
#undef BUILTINS_X_DECL

#define BUILTINS_X_DECL(NAME, TOOL, DSCR) {TOOL##sv, DSCR##sv, NAME::handle},
	static constexpr builtin_tool tools[] = {ROOT_BUILTINS_X(BUILTINS_X_DECL)};

	namespace insured {
		static constexpr builtin_tool tools[] = {
		    INSURED_BUILTINS_X(BUILTINS_X_DECL)};
	}
#undef BUILTINS_X_DECL

	struct help_command {
		std::string_view name;
		std::string_view description;
	};

	struct help_group {
		std::string_view name;
		std::span<help_command const> commands;

		void copy_to(args::chunk& chunk) const;

		static void add_to_parser(args::parser& parser,
		                          std::span<help_group const> groups);
		static void fill_help(args::fmt_list& commands,
		                      std::span<help_group const> groups);
		[[noreturn]] static void show_help(args::parser& parser,
		                                   std::span<help_group const> groups);
	};

	struct options {
		std::string_view tool_name{};
		args::arglist args{};
	};

	class parser {
	public:
		parser(std::string_view description,
		       args::args_view const& args,
		       std::span<help_group const> groups,
		       bool is_root = false);
		options parse_args();
		void noent(std::string_view tool, std::string_view my_name) const;

	protected:
		args::parser& get_parser() noexcept { return parser_; }

	private:
		args::null_translator tr_{};
		args::parser parser_;
		std::span<help_group const> groups_{};
	};

	class root_parser : public parser {
	public:
		root_parser(args::args_view const& args,
		            std::span<help_group const> groups);
	};
}  // namespace quick_dra::builtin
