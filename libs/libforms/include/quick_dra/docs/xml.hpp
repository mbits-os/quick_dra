// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <map>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

using namespace std::literals;

namespace quick_dra {
	struct xml {
		using vector = std::vector<xml>;

		std::string tag;
		std::map<std::string, std::string> attributes;
		std::variant<vector, std::string> inside{};

		bool has_text() const noexcept {
			return std::holds_alternative<std::string>(inside);
		}
		vector& children() { return std::get<vector>(inside); }
		std::string const& text() const {
			return std::get<std::string>(inside);
		}

		xml& with(std::string_view child);
		xml& with(xml&& child);
		xml& with(xml& child);

		void print_open_tag(std::ostream& os) const;
		void print_close_tag(std::ostream& os) const;

		friend std::ostream& operator<<(std::ostream& os, xml const& node);

		struct indented_t {
			xml const& ref{};
			std::string_view indentation{};
			size_t level{};

			indented_t child(xml const& child) const {
				return {child, indentation, level + 1};
			}

			void indent(std::ostream& os) const {
				for (size_t index = 0; index < level; ++index)
					os << indentation;
			}

			friend std::ostream& operator<<(std::ostream& os,
			                                indented_t const& node);
		};

		indented_t indented(std::string_view indentation = "\t"sv) const {
			return {*this, indentation};
		}
	};

	inline xml E(std::string_view const& tag,
	             std::map<std::string, std::string> attributes = {}) {
		return xml{{tag.data(), tag.size()}, attributes};
	}
}  // namespace quick_dra
