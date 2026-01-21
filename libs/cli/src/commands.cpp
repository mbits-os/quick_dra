// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "commands.hpp"
#include <functional>
#include <string>
#include <utility>

using namespace std::literals;

namespace quick_dra {
	namespace {
		std::set<std::string> list_tools(tools const& runner,
		                                 std::string_view group) {
			if (group == "builtins"sv || group == "builtin"sv)
				return runner.list_builtins();
			return {};
		}
	}  // namespace

	int tools::handle(std::string_view tool, args::arglist args) const {
		for (auto const& builtin : builtins_) {
			if (builtin.name != tool) continue;
			static constexpr auto tool_prefix = "qdra "sv;
			std::string name{};
			name.reserve(tool_prefix.size() + tool.size());
			name.append(tool_prefix);
			name.append(tool);
			return builtin.tool(name, args, builtin.description);
		}  // GCOV_EXCL_LINE[WIN32]

		return -ENOENT;
	}

	std::set<std::string> tools::list_builtins() const {
		std::set<std::string, std::less<>> commands{};

		for (auto const& builtin : builtins_) {
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
