// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <array>
#include <quick_dra/base/types.hpp>
#include <quick_dra/docs/xml.hpp>
#include <string>
#include <string_view>
#include <utility>

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
				}  // GCOV_EXCL_LINE[WIN32]
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
				}  // GCOV_EXCL_LINE[WIN32]

				if (!found) result.push_back(c);
			}
			return result;
		}  // GCOV_EXCL_LINE[GCC]
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

	void xml::print_open_tag(std::ostream& os) const {
		fmt::print(os, "<{}", tag);
		for (auto const& [name, value] : attributes) {
			fmt::print(os, " {}=\"{}\"", name, xml_escape(value));
		}
		fmt::print(os, ">");
	}

	void xml::print_close_tag(std::ostream& os) const {
		fmt::print(os, "</{}>", tag);
	}

	std::ostream& operator<<(std::ostream& os, xml const& node) {
		node.print_open_tag(os);

		if (std::holds_alternative<std::string>(node.inside)) {
			fmt::print(os, "{}",
			           xml_escape(std::get<std::string>(node.inside)));
		} else {
			auto const& list = std::get<xml::vector>(node.inside);
			if (list.empty()) {
				os << "<!-- empty -->";
			} else {
				for (auto const& item : list) {
					os << item;
				}
			}
		}

		node.print_close_tag(os);
		return os;
	}

	std::ostream& operator<<(std::ostream& os, xml::indented_t const& node) {
		node.indent(os);
		node.ref.print_open_tag(os);

		if (std::holds_alternative<std::string>(node.ref.inside)) {
			fmt::print(os, "{}",
			           xml_escape(std::get<std::string>(node.ref.inside)));
		} else {
			auto const& list = std::get<xml::vector>(node.ref.inside);
			if (list.empty()) {
				os << "<!-- empty -->";
			} else {
				os << '\n';
				for (auto const& item : list) {
					os << node.child(item);
				}
				node.indent(os);
			}
		}

		node.ref.print_close_tag(os);
		return os << '\n';
	}

}  // namespace quick_dra
