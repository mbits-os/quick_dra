// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <map>
#include <optional>
#include <quick_dra/models/model.hpp>
#include <string>
#include <variant>
#include <vector>

using namespace std::literals;

namespace quick_dra {
	enum class alignement {
		left,
		right,
	};

	struct data_field {
		unsigned number{};
		std::string formatted{};
		std::string label{};
		quick_dra::alignement alignement{};

		auto operator<=>(data_field const&) const noexcept = default;
	};

	struct data_section {
		std::string name{};
		std::string label{};
		std::vector<data_field> fields{};

		auto operator<=>(data_section const&) const noexcept = default;
	};

	struct formatted_report {
		std::string title;
		std::map<std::string, data_section> data{};
		std::vector<std::string> order{};

		auto operator<=>(formatted_report const&) const noexcept = default;

		void add(std::string const& section, data_field const& item, std::map<std::string, std::string> const& labels) {
			auto it = data.lower_bound(section);
			if (it == data.end() || it->first != section) {
				auto label_it = labels.find(section);
				it = data.insert(it, {section,
				                      {
				                          .name = join(split_sv(section, '.'_sep), ". "_sep),
				                          .label = label_it != labels.end() ? label_it->second : ""s,
				                          .fields = {},
				                      }});
				order.push_back(section);
			}
			it->second.fields.push_back(item);
		}
	};

	namespace report_format {
		struct hint {
			std::string sep{" "s};
			std::optional<bool> ignore_{};
			std::optional<quick_dra::alignement> alignment_{};

			static quick_dra::alignement default_alignment_for(percent) { return quick_dra::alignement::right; }
			static quick_dra::alignement default_alignment_for(currency) { return quick_dra::alignement::right; }
			static quick_dra::alignement default_alignment_for(uint_value) { return quick_dra::alignement::right; }
			static quick_dra::alignement default_alignment_for(std::chrono::year_month) {
				return quick_dra::alignement::right;
			}
			static quick_dra::alignement default_alignment_for(std::chrono::year_month_day) {
				return quick_dra::alignement::right;
			}
			static quick_dra::alignement default_alignment_for(auto const&) { return quick_dra::alignement::left; }
			static quick_dra::alignement default_alignment_for(calculated_value const& value) {
				return std::visit([](auto&& val) { return default_alignment_for(val); }, value);
			}

			bool ignore(bool inherited) const noexcept { return ignore_.value_or(inherited); }
			quick_dra::alignement alignment_for(auto&& value) const noexcept {
				if (alignment_) return alignment_.value();
				return default_alignment_for(value);
			}
		};

		struct ref {
			std::string section{};
			std::string block{};
			unsigned field{};

			std::string title_chunk_from(calculated_block const& form_block) const;
			std::string get_chunk(std::vector<calculated_section> const& form) const;
		};

		struct formatting {
			std::vector<ref> title{};
			std::map<std::string, hint> hints{};
			std::map<std::string, std::string> labels{};

			static std::map<std::string, formatting> parse(std::string const& contents, std::string const& path);

			hint const& get_hint(std::string const& key) const noexcept;

			static formatted_report format_report(std::map<std::string, formatting> const&,
			                                      std::string const& key,
			                                      std::vector<calculated_section> const& form);

		private:
			std::string get_title(std::vector<calculated_section> const& form) const;

			void visit_block(calculated_block const& block,
			                 bool inherited_ignore,
			                 std::string const& section,
			                 formatted_report& report) const;
			void visit_section(calculated_section const& section, formatted_report& report) const;
			formatted_report format(std::string const& key, std::vector<calculated_section> const& form) const;
		};
	}  // namespace report_format
}  // namespace quick_dra
