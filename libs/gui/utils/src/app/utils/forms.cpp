// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <app/utils/forms.hpp>
#include <array>
#include <numeric>
#include <quick_dra/base/chrono.hpp>
#include <quick_dra/base/str.hpp>
#include <string>
#include <string_view>

using namespace std::literals;

namespace quick_dra::gui {
	namespace {
		static constexpr const auto document_kinds = std::array{
		    std::pair{'R', "REGON"sv},
		    std::pair{'P', "PESEL"sv},
		    std::pair{'1', "dowód osobisty"sv},
		    std::pair{'2', "paszport"sv},
		};

		static constexpr const auto fractions = std::array{
		    std::pair{"1/4"sv, u8"\u00bc"sv}, std::pair{"1/2"sv, u8"\u00bd"sv}, std::pair{"3/4"sv, u8"\u00be"sv},
		    std::pair{"1/7"sv, u8"\u2150"sv}, std::pair{"1/9"sv, u8"\u2151"sv}, std::pair{"1/10"sv, u8"\u2152"sv},
		    std::pair{"1/3"sv, u8"\u2153"sv}, std::pair{"2/3"sv, u8"\u2154"sv}, std::pair{"1/5"sv, u8"\u2155"sv},
		    std::pair{"2/5"sv, u8"\u2156"sv}, std::pair{"3/5"sv, u8"\u2157"sv}, std::pair{"4/5"sv, u8"\u2158"sv},
		    std::pair{"1/6"sv, u8"\u2159"sv}, std::pair{"5/6"sv, u8"\u215A"sv}, std::pair{"1/8"sv, u8"\u215B"sv},
		    std::pair{"3/8"sv, u8"\u215C"sv}, std::pair{"5/8"sv, u8"\u215D"sv}, std::pair{"7/8"sv, u8"\u215E"sv},
		};
	}  // namespace

	std::string ratio_from(unsigned num, unsigned den) {
		auto const value = std::gcd(num, den);
		num /= value;
		den /= value;
		auto const ascii = fmt::format("{}/{}", num, den);
		for (auto const& [key, translation] : fractions) {
			if (key == ascii) {
				return as_str(translation);
			}
		}
		return ascii;
	}

	std::string_view document_kind(char kind) {
		for (auto const& [doc_kind, name] : document_kinds) {
			if (doc_kind == kind) {
				return name;
			}
		}
		return "nieznany typ"sv;
	}

	std::string info_span(std::string_view label, std::string_view value) {
		return fmt::format("*{}:* {}", label, value);
	}

	std::string document_info(char kind, std::optional<std::string> const& document) {
		if (!document || document->empty()) return {};
		return info_span(document_kind(kind), *document);
	}

	std::string document_info(std::optional<std::string> const& kind, std::optional<std::string> const& document) {
		return document_info(kind && !kind->empty() ? kind->front() : '\0', document);
	}

	std::string insurance_title_info(std::optional<insurance_title> const& title) {
		return title ? info_span("tytuł", fmt::to_string(*title)) : std::string{};
	}

	std::string salary_info(year_month const& month,
	                        std::optional<ratio> const& scale,
	                        std::optional<currency> const& salary) {
		static constexpr auto salary_label = "pensja"sv;

		auto const [num, den] = scale.value_or(ratio{1, 1});
		auto const simple = num == den;

		if (salary) {
			auto const full = *salary;
			return simple
			           ? info_span(salary_label, fmt::format("{} zł", full))
			           : info_span(salary_label, fmt::format("{} zł ({} z {} zł)",
			                                                 calc_currency{(full.calc().value * num) / den}.rounded(),
			                                                 ratio_from(num, den), full));
		}

		std::string since{};
		if (month != null_month) {
			since = fmt::format(" (od {}/{:02})", static_cast<int>(month.year()), static_cast<unsigned>(month.month()));
		}

		return simple ? info_span(salary_label, fmt::format("minimalna{}", since))
		              : info_span(salary_label, fmt::format("{} minimalnej{}", ratio_from(num, den), since));
	}

	std::string name_from(std::optional<std::string> const& first_name,
	                      std::optional<std::string> const& last_name,
	                      bool markdown) {
		static constexpr auto markdown_bold = "**"sv;
		static constexpr auto text_markers = "<>"sv;
		auto const markers = markdown ? markdown_bold : text_markers;

		if (!first_name && !last_name) {
			return fmt::format("{}Nazwisko nieznane{}", markers[0], markers[1]);
		}
		if (!first_name) {
			return fmt::format("{}???{} {}", markers[0], markers[1], *last_name);
		}
		if (!last_name) {
			return fmt::format("{} {}???{}", *first_name, markers[0], markers[1]);
		}
		return fmt::format("{} {}", *first_name, *last_name);
	}
}  // namespace quick_dra::gui
