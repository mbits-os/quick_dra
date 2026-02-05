// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "commands.hpp"
#include <functional>
#include <string>
#include <utility>

using namespace std::literals;

namespace quick_dra {
	int tools::handle(std::string_view tool,
	                  args::arglist args,
	                  std::string_view tool_name) const {
		for (auto const& command : commands_) {
			if (command.name != tool) continue;
			std::string name{};
			name.reserve(tool_name.size() + tool.size() + 1);
			name.append(tool_name);
			name.push_back(' ');
			name.append(tool);
			return command.tool(name, args, command.description);
		}  // GCOV_EXCL_LINE[WIN32]

		return -ENOENT;
	}

	std::set<std::string> tools::list_commands() const {
		std::set<std::string, std::less<>> commands{};

		for (auto const& builtin : commands_) {
			auto it = commands.lower_bound(builtin.name);
			if (it == commands.end() || *it != builtin.name)
				commands.insert(
				    it, std::string{builtin.name.data(), builtin.name.size()});
		}

		std::set<std::string> result{};
		result.merge(std::move(commands));
		return result;
	}
}  // namespace quick_dra
