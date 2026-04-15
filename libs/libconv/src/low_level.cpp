// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <istream>
#include <quick_dra/conv/low_level.hpp>
#include <string>
#include <utility>

namespace quick_dra {
	using std::literals::operator""s;
	using std::literals::operator""sv;

	void comment(std::string_view msg) { fmt::print("\033[0;90m - {}\033[m\n", msg); }

	bool get_answer(std::string_view label,
	                std::string_view hint,
	                std::function<bool(std::string&&)> const& answer_is_valid,
	                std::istream& in) {
		auto const hint_brackets = hint.empty() ? ""s : fmt::format("\033[0;90m [{}]", hint);

		while (true) {
			std::string answer;
			fmt::print("\033[0;36m{}{}\033[m> ", label, hint_brackets);
			if (!std::getline(in, answer)) {
				return false;
			}
			if (answer_is_valid(std::move(answer))) {
				return true;
			}
		}
	}  // GCOV_EXCL_LINE

	bool get_yes_no(std::string_view label, bool hint, bool& dst, std::istream& in) {
		auto const handler = [&, hint](std::string const& input) -> bool {
			if (input.empty()) {
				dst = hint;
				return true;
			}
			if (input == "y"sv || input == "Y"sv) {
				dst = true;
				return true;
			}
			if (input == "n"sv || input == "N"sv) {
				dst = false;
				return true;
			}
			return false;
		};

		return get_answer(label, hint ? "Y/n"sv : "y/N"sv, handler, in);
	}

	bool get_enum_answer(std::string_view label,
	                     std::span<std::pair<char, std::string_view> const> items,
	                     std::function<void(char)> const& store_enum,
	                     char selected,
	                     std::istream& in) {
		std::string hint{};
		hint.reserve([&items]() {
			size_t hint_size = items.size() * 4 + (items.size() - 1) * 2;
			for (auto const& [_, description] : items) {
				hint_size += description.size();
			}
			return hint_size;
		}() + static_cast<size_t>(selected ? 2u : 0u));

		for (auto const& [value, description] : items) {
			if (!hint.empty()) hint.append(", "sv);
			if (value == selected) hint.push_back('[');
			hint.push_back(value);
			if (value == selected) hint.push_back(']');
			hint.append(" - "sv);
			hint.append(description);
		}

		return get_answer(
		    label, hint,
		    [&, selected](std::string const& answer) {
			    if (selected && answer.empty()) {
				    store_enum(selected);
				    return true;
			    }

			    if (answer.size() != 1) {
				    return false;
			    }

			    for (auto const& [value, _] : items) {
				    if (value == answer.front()) {
					    store_enum(value);
					    return true;
				    }
			    }

			    return false;
		    },
		    in);
	}

	std::string as_string(insurance_title const& value) { return fmt::to_string(fmt::join(value.split(), " ")); }

	std::string as_string(ratio const& value) { return fmt::format("{}/{}", value.num, value.den); }

	std::string as_string(currency const& value) {
		if (value == minimal_salary) {
			return "minimal for a given month"s;
		}
		return fmt::format("{} zł", value);
	}

	std::optional<std::string> as_string(std::optional<currency> const& value) {
		return value.transform([](auto const& value) { return as_string(value); }).value_or("none"s);
	}

	template <typename Arg>
	bool get_field_answer_impl(bool ask_questions,
	                           std::string_view label,
	                           std::optional<Arg>& dst,
	                           std::optional<Arg>&& opt,
	                           std::function<bool(std::string&&, std::optional<Arg>&, bool)> const& validator,
	                           std::istream& in) {
		if (opt) {
			dst = std::move(opt);
		}

		if (!ask_questions) {
			if (dst) {
				auto copy = *dst;
				auto const is_valid = validator(as_string(std::move(copy)), dst, false);
				if (!is_valid) {
					comment("Cannot save invalid data with -y. Stopping.");
					return false;
				}
			}  // GCOV_EXCL_LINE[WIN32]
			if constexpr (std::same_as<Arg, currency>) {
				if (!dst) {
					auto const is_valid = validator("none"s, dst, false);
					if (!is_valid) {
						comment("Cannot save invalid data with -y. Stopping.");
						return false;
					}
				}
			}
			return true;
		}

		auto def_value = as_string(dst);
		return get_answer(
		    label, def_value.value_or(""s),
		    [&](std::string&& answer) {
			    if (answer.empty() && def_value) {
				    answer = *def_value;
			    }
			    return validator(std::move(answer), dst, true);
		    },
		    in);
	}

	bool get_field_answer(bool ask_questions,
	                      std::string_view label,
	                      std::optional<std::string>& dst,
	                      std::optional<std::string>&& opt,
	                      std::function<bool(std::string&&, std::optional<std::string>&, bool)> const& validator,
	                      std::istream& in) {
		return get_field_answer_impl(ask_questions, label, dst, std::move(opt), validator, in);
	}

	bool get_field_answer(bool ask_questions,
	                      std::string_view label,
	                      std::optional<insurance_title>& dst,
	                      std::optional<insurance_title>&& opt,
	                      std::function<bool(std::string&&, std::optional<insurance_title>&, bool)> const& validator,
	                      std::istream& in) {
		return get_field_answer_impl(ask_questions, label, dst, std::move(opt), validator, in);
	}

	static constexpr auto magic_currency = "minimal"sv;
	bool get_field_answer(bool ask_questions,
	                      std::string_view label,
	                      std::optional<currency>& dst,
	                      std::optional<currency>&& opt,
	                      std::function<bool(std::string&&, std::optional<currency>&, bool)> const& validator,
	                      std::istream& in) {
		std::function<bool(std::string&&, std::optional<currency>&, bool)> const wrapped =
		    [validator](std::string&& in, std::optional<currency>& out, bool ask_questions) {
			    if (in.starts_with(magic_currency)) {
				    in = magic_currency;
			    }
			    return validator(std::move(in), out, ask_questions);
		    };
		return get_field_answer_impl(ask_questions, label, dst, std::move(opt), wrapped, in);
	}

	bool get_field_answer(bool ask_questions,
	                      std::string_view label,
	                      std::optional<ratio>& dst,
	                      std::optional<ratio>&& opt,
	                      std::function<bool(std::string&&, std::optional<ratio>&, bool)> const& validator,
	                      std::istream& in) {
		return get_field_answer_impl(ask_questions, label, dst, std::move(opt), validator, in);
	}
}  // namespace quick_dra
