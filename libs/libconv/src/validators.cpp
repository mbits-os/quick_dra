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

	bool valid_tax_id(std::string&& value,
	                  std::optional<std::string>& dst,
	                  bool) {
		std::string non_dashed{};
		non_dashed.reserve(value.size());
		for (auto const ch : value) {
			if (ch == '-') continue;
			non_dashed.push_back(ch);
		}
		if (!tax_id_validator::is_valid(non_dashed)) {
			comment("The tax id provided seems to be invalid.");
			return false;
		}
		dst = std::move(non_dashed);
		return true;
	}

	bool valid_title(std::string&& value,
	                 std::optional<insurance_title>& dst,
	                 bool) {
		insurance_title title{};
		if (!insurance_title::parse(value, title)) {
			comment("The insurance title value provided seems to be invalid.");
			return false;
		}

		dst = title;
		return true;
	}

	bool valid_part_time_scale(std::string&& value,
	                           std::optional<ratio>& dst,
	                           bool) {
		ratio scale{};
		if (!ratio::parse(value, scale)) {
			comment("The ratio value provided seems to be invalid.");
			return false;
		}

		dst = scale;
		return true;
	}

	bool valid_salary(std::string&& value, std::optional<currency>& dst, bool) {
		using std::literals::operator""sv;

		if (value == "minimal"sv) {
			dst = currency{-1 * currency::den};
			return true;
		}

		currency money{};
		if (!currency::parse(value, money)) {
			comment("The monetary value provided seems to be invalid.");
			return false;
		}

		dst = money;
		return true;
	}
}  // namespace quick_dra::builtin
