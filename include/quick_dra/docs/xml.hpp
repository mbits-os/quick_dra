// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <map>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace quick_dra {
	struct xml {
		using vector = std::vector<xml>;

		std::string tag;
		std::map<std::string, std::string> attributes;
		std::variant<vector, std::string> inside{};

		vector& children() { return std::get<vector>(inside); }
		std::string const& text() const {
			return std::get<std::string>(inside);
		}

		xml& with(std::string_view child);
		xml& with(xml&& child);
		xml& with(xml& child);

		friend std::ostream& operator<<(std::ostream& os, xml const& node);
	};

	inline xml E(std::string_view const& tag,
	             std::map<std::string, std::string> attributes = {}) {
		return xml{{tag.data(), tag.size()}, attributes};
	}
}  // namespace quick_dra
