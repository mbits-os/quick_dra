// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <quick_dra/conv/low_level.hpp>
#include <quick_dra/conv/validators.hpp>
#include <quick_dra/lex/validators.hpp>
#include <utility>

namespace quick_dra::builtin {
	namespace {
		bool valid_nonempty_string(std::string&& value,
		                           std::optional<std::string>& dst) {
			if (value.empty()) return false;
			dst = std::move(value);
			return true;
		}
	}  // namespace

	bool valid_first_name(std::string&& value,
	                      std::optional<std::string>& dst,
	                      bool) {
		return valid_nonempty_string(std::move(value), dst);
	}
	bool valid_last_name(std::string&& value,
	                     std::optional<std::string>& dst,
	                     bool) {
		return valid_nonempty_string(std::move(value), dst);
	}

	bool valid_tax_id(std::string&& value,
	                  std::optional<std::string>& dst,
	                  bool) {
		if (!tax_id_validator::is_valid(value)) {
			comment("The tax id provided seems to be invalid.");
			return false;
		}
		dst = std::move(value);
		return true;
	}

	bool valid_social_id(std::string&& value,
	                     std::optional<std::string>& dst,
	                     bool) {
		if (!social_id_validator::is_valid(value)) {
			comment("The social id provided seems to be invalid.");
			return false;
		}
		dst = std::move(value);
		return true;
	}

	bool valid_id_card(std::string&& value,
	                   std::optional<std::string>& dst,
	                   bool) {
		if (!id_card_validator::is_valid(value)) {
			comment("The ID card provided seems to be invalid.");
			return false;
		}
		dst = std::move(value);
		return true;
	}

	bool valid_passport(std::string&& value,
	                    std::optional<std::string>& dst,
	                    bool) {
		if (!pl_passport_validator::is_valid(value)) {
			comment("The passport number provided seems to be invalid.");
			return false;
		}
		dst = std::move(value);
		return true;
	}
}  // namespace quick_dra::builtin
