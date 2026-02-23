// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <array>
#include <map>
#include <quick_dra/models/project_reader.hpp>
#include <ranges>
#include <span>
#include <utility>
#include <variant>
#include <vector>
#include <yaml/parser.hpp>

namespace quick_dra {
	inline std::optional<std::string> as_str(
	    std::optional<std::string_view> view) {
		if (!view) return std::nullopt;
		return as_str(*view);
	}
};  // namespace quick_dra

namespace quick_dra::testing {
	template <typename FileObj>
	class object_reader {
	public:
		static constexpr std::string_view empty_log{};
		std::string log{};

		std::optional<FileObj> read(std::string_view text) {
			::testing::internal::CaptureStderr();
			auto result = parser::parse_yaml_text<FileObj>(
			    {text.data(), text.size()}, "input"s);
			log = ::testing::internal::GetCapturedStderr();
			return result;
		}

		std::string write(FileObj const& obj) {
			ryml::Tree tree{};
			auto ref = tree.rootref();
			yaml::write_value(ref, obj);

			ryml::csubstr output =
			    ryml::emit_yaml(tree, tree.root_id(), ryml::substr{},
			                    /*error_on_excess*/ false);

			std::vector<char> buf(output.len);
			output = ryml::emit_yaml(tree, tree.root_id(), ryml::to_substr(buf),
			                         /*error_on_excess*/ true);

			return {output.data(), output.size()};
		}
	};

	using templates_field_value =
	    std::variant<std::string, std::vector<std::string>>;
	using templates_fields = std::map<unsigned, templates_field_value>;
	using templates_report = std::vector<report_section>;
	using templates_reports = std::map<std::string, templates_report>;
}  // namespace quick_dra::testing
