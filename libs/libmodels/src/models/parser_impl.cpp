// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <cmath>
#include <quick_dra/base/str.hpp>
#include <quick_dra/models/project_reader.hpp>

namespace quick_dra::v1 {
	namespace {
		static constexpr auto app_name = "Quick-DRA"sv;
	}
	std::optional<config> config::parse_yaml(
	    std::filesystem::path const& path) {
		return parser::parse_yaml_file<config>(path, app_name);
	}

	std::optional<std::map<std::chrono::year_month, currency>>
	config::parse_minimal(std::filesystem::path const& path) {
		return parser::parse_yaml_file<
		    std::map<std::chrono::year_month, currency>>(path, app_name);
	}

	std::optional<std::map<std::chrono::year_month, currency>>
	config::parse_minimal_from_text(std::string const& text,
	                                std::string const& path) {
		return parser::parse_yaml_text<
		    std::map<std::chrono::year_month, currency>>(text, path);
	}

	bool config::postprocess() { return version == kVersion; }

	template <typename Named>
	bool parse_and_validate_name(Named& named) noexcept {
		auto const name = split_sv(named.last_name, ", "_sep, 1);
		if (name.size() != 2) return false;
		named.first_name = strip_sv(name[1]);
		named.last_name = strip_sv(name[0]);
		return !(named.last_name.empty() || named.first_name.empty());
	}

	bool insurer_t::postprocess() {
		kind.clear();
		document.clear();

		if (id_card) {
			if (passport) {
				return false;
			}
			kind = "1"sv;
			document = std::move(*id_card);
		}

		if (passport) {
			kind = "2"sv;
			document = std::move(*passport);
		}

		return parse_and_validate_name(*this) &&
		       !(social_id.empty() || tax_id.empty() || kind.empty() ||
		         document.empty());
	}

	bool insured_t::postprocess() {
		if (!parse_and_validate_name(*this)) return false;

		if (title.code.length() != 8) return false;
		if (!(std::isdigit(title.code[0]) && std::isdigit(title.code[1]) &&
		      std::isdigit(title.code[2]) && std::isdigit(title.code[3]) &&
		      std::isdigit(title.code[5]) && std::isdigit(title.code[7]) &&
		      title.code[4] == ' ' && title.code[6] == ' ')) {
			return false;
		}

		auto const view = std::string_view{title.code};
		title.code = fmt::format("{}{}{}", view.substr(0, 4), view[5], view[7]);

		kind.clear();
		document.clear();

		if (social_id) {
			if (id_card || passport) {
				return false;
			}
			kind = "P"sv;
			document = std::move(*social_id);
		}

		if (id_card) {
			if (social_id || passport) {
				return false;
			}
			kind = "1"sv;
			document = std::move(*id_card);
		}

		if (passport) {
			if (social_id || id_card) {
				return false;
			}
			kind = "2"sv;
			document = std::move(*passport);
		}

		if (kind.empty() || document.empty()) return false;
		return true;
	}

	std::optional<templates> templates::parse_yaml(
	    std::filesystem::path const& path) {
		return parser::parse_yaml_file<templates>(path, app_name);
	}

	bool templates::validate() noexcept {
		for (auto& [kedu, report] : reports) {
			if (kedu.empty()) return false;
			for (auto& section : report) {
				if (!section.validate()) return false;
			}
		}
		return true;
	}

	bool report_section::validate() noexcept {
		if (id.empty()) return false;

		struct validator {
			bool operator()(std::string const& str) const noexcept {
				return !str.empty();
			}
			bool operator()(
			    std::vector<std::string> const& vec) const noexcept {
				if (vec.empty()) return false;
				for (auto const& str : vec) {
					if (str.empty()) return false;
				}
				return true;
			}
		} value{};

		for (auto const& [_, field] : fields) {
			if (!std::visit(value, field)) return false;
		}
		return true;
	}

	contribution rate::contribution_on(currency amount) const noexcept {
		return contribution_on(amount.calc());
	}

	contribution rate::contribution_on(calc_currency amount) const noexcept {
		auto const total_contribution = (amount * total).rounded();
		if (!insured) {
			return {.payer = total_contribution, .insured = {}};
		}
		auto const insured_contribution = (amount * *insured).rounded();
		return {.payer = total_contribution - insured_contribution,
		        .insured = insured_contribution};
	}
}  // namespace quick_dra::v1
