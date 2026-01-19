// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <numeric>
#include <quick_dra/docs/forms.hpp>
#include <quick_dra/docs/summary.hpp>
#include <ranges>

namespace quick_dra {
	namespace {
		template <typename T>
		std::optional<T> get_typed_value(global_object const& root,
		                                 varname const& ref) {
			auto const ptr = root.peek(ref);
			if (!ptr || !ptr->value) {
				return std::nullopt;
			}
			auto const scalar =
			    std::get_if<calculated_value>(std::addressof(*ptr->value));
			auto const data = std::get_if<T>(scalar);
			return data ? std::optional{*data} : std::nullopt;
		}

		size_t codepoints(std::string_view utf8) {
			size_t len = 0;
			for (auto const c : utf8) {
				len += static_cast<int>(
				    (static_cast<unsigned char>(c) & 0xC0) != 0x80);
			}
			return len;
		}

		std::string to_string(currency const& v) {
			return fmt::format("{} zł", v);
		}

		std::string format(std::optional<currency> const& value) {
			return value.transform(to_string).value_or("(nieznane)"s);
		}
	};  // namespace

	std::vector<summary_line> gather_summary_data(
	    std::vector<quick_dra::form> const& forms) {
		size_t rows = 2;
		for (auto const& form : forms) {
			if (form.key != "RCA"s) continue;
			++rows;
		}

		std::vector<summary_line> result{};
		result.reserve(rows);

		auto insurance_total = std::optional<currency>{};
		auto tax_total = std::optional<currency>{};

		for (auto const& form : forms) {
			if (form.key == "DRA"s) {
				insurance_total =
				    get_typed_value<currency>(form.state, var::insurance_total);
				tax_total =
				    get_typed_value<currency>(form.state, var::tax_total);
			}
			if (form.key != "RCA"s) continue;

			auto first_name =
			    get_typed_value<std::string>(form.state, var::insured.first)
			        .value_or("<brak imienia>"s);
			auto last_name =
			    get_typed_value<std::string>(form.state, var::insured.last)
			        .value_or("<brak nazwiska>"s);
			auto const net_amount =
			    get_typed_value<currency>(form.state, var::remuneration.net);

			result.push_back(
			    {.label = fmt::format("{} {}", first_name, last_name),
			     .value = net_amount});
		}

		result.push_back({.label = "ZUS"s, .value = insurance_total});
		result.push_back({.label = "Urząd Skarbowy"s, .value = tax_total});

		return result;
	}

	void print_summary(std::vector<summary_line> const& rows) {
		auto const total_range =
		    rows | std::views::transform([](auto const& line) {
			    return line.value.value_or(currency{});
		    });
		auto const total = std::reduce(
		    total_range.begin(), total_range.end(), currency{},
		    [](auto const& lhs, auto const& rhs) { return lhs + rhs; });

		auto lines = rows | std::views::transform([](auto const& line) {
			             return std::pair{fmt::format("- {}:", line.label),
			                              format(line.value)};
		             }) |
		             std::ranges::to<std::vector>();
		lines.emplace_back("sum total ="s, to_string(total));

		auto const [labels, values] = std::transform_reduce(
		    lines.begin(), lines.end(), std::pair<size_t, size_t>{},
		    // reduce: max codepoint count per column
		    [](auto const& lhs, auto const& rhs) {
			    return std::pair{std::max(lhs.first, rhs.first),
			                     std::max(lhs.second, rhs.second)};
		    },
		    // transform: codepoint count in string
		    [](auto const& line) {
			    return std::pair{codepoints(line.first),
			                     codepoints(line.second)};
		    });

		fmt::print("-- payments:\n");
		for (auto const& [label, value] : lines) {
			fmt::print("   {:<{}} {:>{}}\n", label, labels, value, values);
		}
	}
}  // namespace quick_dra
