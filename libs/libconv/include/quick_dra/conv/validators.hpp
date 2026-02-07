// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <optional>
#include <quick_dra/conv/field_policy.hpp>
#include <string>

namespace quick_dra::builtin {
	bool valid_first_name(std::string&& value,
	                      std::optional<std::string>& dst,
	                      bool);
	bool valid_last_name(std::string&& value,
	                     std::optional<std::string>& dst,
	                     bool);
	bool valid_social_id(std::string&& value,
	                     std::optional<std::string>& dst,
	                     bool);
	bool valid_id_card(std::string&& value,
	                   std::optional<std::string>& dst,
	                   bool);
	bool valid_passport(std::string&& value,
	                    std::optional<std::string>& dst,
	                    bool);
	bool valid_tax_id(std::string&& value,
	                  std::optional<std::string>& dst,
	                  bool);
	bool valid_title(std::string&& value,
	                 std::optional<insurance_title>& dst,
	                 bool);
	bool valid_part_time_scale(std::string&& value,
	                           std::optional<ratio>& dst,
	                           bool);
	bool valid_salary(std::string&& value, std::optional<currency>& dst, bool);

	namespace policies {
		template <typename T>
		static inline bool always_valid(std::string&&,
		                                std::optional<T>&,
		                                bool) {
			return true;
		}

		static constexpr auto first_name =
		    "First name"_label / getters::first_name{} / valid_first_name;
		static constexpr auto last_name =
		    "Last name"_label / getters::last_name{} / valid_last_name;
		static constexpr auto social_id =
		    "PESEL"_label / getters::social_id{} / valid_social_id;
		static constexpr auto id_card =
		    "ID card"_label / getters::id_card{} / valid_id_card;
		static constexpr auto passport =
		    "Passport"_label / getters::passport{} / valid_passport;
		static constexpr auto kind =
		    "Document kind"_label / getters::kind{} / always_valid<std::string>;
		static constexpr auto document =
		    "Document"_label / getters::document{} / always_valid<std::string>;

		static constexpr auto tax_id =
		    "NIP"_label / getters::tax_id{} / valid_tax_id;

		static constexpr auto title =
		    "Insurance title"_label / getters::title{} / valid_title;
		static constexpr auto part_time_scale = "Part-time scale"_label /
		                                        getters::part_time_scale{} /
		                                        valid_part_time_scale;
		static constexpr auto salary =
		    "Salary"_label / getters::salary{} / valid_salary;
	}  // namespace policies
}  // namespace quick_dra::builtin
