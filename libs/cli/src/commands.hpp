// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <args_parser.hpp>
#include <set>
#include <span>
#include <string>

namespace quick_dra {
	struct builtin_tool {
		std::string_view name;
		std::string_view description;
		int (*tool)(std::string_view tool,
		            args::arglist args,
		            std::string_view description);
	};

	class tools {
	public:
		tools(std::span<builtin_tool const> const& commands)
		    : commands_{commands} {}

		int handle(std::string_view tool,
		           args::arglist tool_args,
		           std::string_view parent_name) const;

		std::set<std::string> list_commands() const;

		template <typename ArgParser>
		static int run(ArgParser& parser,
		               std::span<builtin_tool const> const& commands,
		               std::string_view parent_name) {
			auto const [tool, tool_args] = parser.parse_args();

			auto const ret =
			    tools{commands}.handle(tool, tool_args, parent_name);
			if (ret == -ENOENT) {
				parser.noent(tool, parent_name);
				return 1;
			}

			if (ret < 0) return -ret;
			return ret;
		}

	private:
		std::span<builtin_tool const> commands_;
	};
}  // namespace quick_dra
