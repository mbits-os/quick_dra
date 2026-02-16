// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <fmt/std.h>
#include <c4/yml/node.hpp>
#include <fstream>
#include <optional>
#include <string>
#include <yaml/writer.hpp>

using namespace std::literals;

namespace yaml {
	void write_value(ryml::NodeRef& ref, std::string const& ctx) {
		ref << c4::csubstr{ctx.c_str(), ctx.length()};
	}
}  // namespace yaml
