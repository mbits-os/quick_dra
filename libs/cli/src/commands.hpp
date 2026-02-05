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
		tools(std::span<builtin_tool const> const& builtins)
		    : builtins_{builtins} {}

		int handle(std::string_view tool, args::arglist args) const;

		std::set<std::string> list_builtins() const;

	private:
		std::span<builtin_tool const> builtins_;
	};
}  // namespace quick_dra
