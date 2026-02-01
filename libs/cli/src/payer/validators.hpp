// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <optional>
#include <quick_dra/conv/field_policy.hpp>
#include <string>
#include <utility>

namespace quick_dra::builtin::payer {
	bool valid_first_name(std::string&& value,
	                      std::optional<std::string>& dst,
	                      bool);
	bool valid_last_name(std::string&& value,
	                     std::optional<std::string>& dst,
	                     bool);
	bool valid_tax_id(std::string&& value,
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

	namespace policies {
#define DECLARE_SELECTOR_EX(NAME, LABEL, VALIDATOR) \
	static constexpr auto NAME = LABEL##_label.with<getters::NAME>() / VALIDATOR

#define DECLARE_SELECTOR(NAME, LABEL)                                  \
	DECLARE_SELECTOR_EX(                                               \
	    NAME, LABEL,                                                   \
	    ([](std::string&& value, std::optional<std::string>& dst,      \
	        bool ask_questions) {                                      \
		    return valid_##NAME(std::move(value), dst, ask_questions); \
	    }))
		DECLARE_SELECTOR(first_name, "First name");
		DECLARE_SELECTOR(last_name, "Last name");
		DECLARE_SELECTOR(tax_id, "NIP");
		DECLARE_SELECTOR(social_id, "PESEL");
		DECLARE_SELECTOR(id_card, "ID card");
		DECLARE_SELECTOR(passport, "Passport");
		DECLARE_SELECTOR_EX(kind, "Document kind", [](auto&...) {
			return true;
		});
		DECLARE_SELECTOR_EX(document, "Document", [](auto&...) {
			return true;
		});
	}  // namespace policies
}  // namespace quick_dra::builtin::payer
