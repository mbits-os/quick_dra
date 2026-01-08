// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <array>
#include <quick_dra/docs/xml.hpp>
#include <quick_dra/model/base_types.hpp>
#include <string>
#include <string_view>

using namespace std::literals;

namespace quick_dra {
	namespace {
		static constexpr auto entities = std::array{
		    std::pair{'&', "&amp;"sv},  std::pair{'<', "&lt;"sv},
		    std::pair{'>', "&gt;"sv},   std::pair{'"', "&quot;"sv},
		    std::pair{'\'', "&#39;"sv},
		};

		std::string xml_escape(std::string_view value) {
			size_t length = value.size();
			for (auto const c : value) {
				for (auto const& [key, esc] : entities) {
					if (c != key) continue;
					length += esc.length() - 1;
					break;
				}
			}

			std::string result{};
			result.reserve(length);
			for (auto const c : value) {
				bool found = false;
				for (auto const& [key, esc] : entities) {
					if (c != key) continue;
					result.append(esc);
					found = true;
					break;
				}
				if (!found) result.push_back(c);
			}
			return result;
		}
	}  // namespace

	xml& xml::with(std::string_view child) {
		inside = std::string{child.data(), child.size()};
		return *this;
	}

	xml& xml::with(xml&& child) {
		if (!std::holds_alternative<vector>(inside)) {
			inside = vector{};
		}
		std::get<vector>(inside).push_back(std::move(child));
		return *this;
	}

	xml& xml::with(xml& child) {
		if (!std::holds_alternative<vector>(inside)) {
			inside = vector{};
		}
		std::get<vector>(inside).push_back(std::move(child));
		return *this;
	}

	std::ostream& operator<<(std::ostream& os, xml const& node) {
		fmt::print(os, "<{}", node.tag);
		for (auto const& [name, value] : node.attributes) {
			fmt::print(os, " {}=\"{}\"", name, xml_escape(value));
		}

		fmt::print(os, ">");

		if (std::holds_alternative<std::string>(node.inside)) {
			fmt::print(os, "{}</{}>",
			           xml_escape(std::get<std::string>(node.inside)),
			           node.tag);
			return os;
		}

		auto const& list = std::get<xml::vector>(node.inside);
		if (list.empty()) {
			fmt::print(os, "<!-- empty --></{}>", node.tag);
			return os;
		}

		for (auto const& item : list) {
			os << item;
		}

		fmt::print(os, "</{}>", node.tag);
		return os;
	}
}  // namespace quick_dra
