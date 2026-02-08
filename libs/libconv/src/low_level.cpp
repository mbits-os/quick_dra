// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <iostream>
#include <quick_dra/conv/low_level.hpp>

namespace quick_dra {
	using std::literals::operator""s;
	using std::literals::operator""sv;

	void comment(std::string_view msg) {
		fmt::print("\033[0;90m - {}\033[m\n", msg);
	}

	bool get_answer(std::string_view label,
	                std::string_view hint,
	                std::function<bool(std::string&&)> const& answer_is_valid) {
		auto const hint_brackets =
		    hint.empty() ? ""s : fmt::format("\033[0;90m [{}]", hint);

		while (true) {
			std::string answer;
			fmt::print("\033[0;36m{}{}\033[m> ", label, hint_brackets);
			if (!std::getline(std::cin, answer)) {
				return false;
			}
			if (answer_is_valid(std::move(answer))) {
				return true;
			}
		}
	}

	bool get_enum_answer(
	    std::string_view label,
	    std::span<std::pair<char, std::string_view> const> const& items,
	    std::function<void(char)> const& store_enum,
	    char selected) {
		std::string hint{};
		hint.reserve([&items]() {
			size_t hint_size = items.size() * 4 + (items.size() - 1) * 2;
			for (auto const& [_, description] : items) {
				hint_size += description.size();
			}
			return hint_size;
		}() + selected
		                 ? 2
		                 : 0);

		for (auto const& [value, description] : items) {
			if (!hint.empty()) hint.append(", "sv);
			if (value == selected) hint.push_back('[');
			hint.push_back(value);
			if (value == selected) hint.push_back(']');
			hint.append(" - "sv);
			hint.append(description);
		}

		return get_answer(label, hint, [&, selected](std::string&& answer) {
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
		});
	}

	std::string as_string(insurance_title const& value) {
		return fmt::to_string(fmt::join(value.split(), " "));
	}

	std::string as_string(ratio const& value) {
		return fmt::format("{}/{}", value.num, value.den);
	}

	std::string as_string(currency const& value) {
		if (value < currency{}) {
			return "minimal for a given month"s;
		}
		return fmt::format("{} zł", value);
	}

	std::optional<std::string> as_string(std::optional<currency> const& value) {
		return value
		    .transform([](auto const& value) { return as_string(value); })
		    .value_or("none"s);
	}

	template <typename Arg>
	bool get_field_answer_impl(
	    bool ask_questions,
	    std::string_view label,
	    std::optional<Arg>& dst,
	    std::optional<Arg>&& opt,
	    std::function<bool(std::string&&, std::optional<Arg>&, bool)> const&
	        validator) {
		if (opt) {
			dst = std::move(opt);
		}

		if (!ask_questions) {
			if (dst) {
				auto copy = *dst;
				auto const is_valid =
				    validator(as_string(std::move(copy)), dst, false);
				if (!is_valid) {
					comment("Cannot save invalid data with -y. Stopping.");
					return false;
				}
			}
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
		return get_answer(label, def_value.value_or(""s),
		                  [&](std::string&& answer) {
			                  if (answer.empty() && def_value) {
				                  answer = *def_value;
			                  }
			                  return validator(std::move(answer), dst, true);
		                  });
	}

	bool get_field_answer(bool ask_questions,
	                      std::string_view label,
	                      std::optional<std::string>& dst,
	                      std::optional<std::string>&& opt,
	                      std::function<bool(std::string&&,
	                                         std::optional<std::string>&,
	                                         bool)> const& validator) {
		return get_field_answer_impl(ask_questions, label, dst, std::move(opt),
		                             validator);
	}

	bool get_field_answer(bool ask_questions,
	                      std::string_view label,
	                      std::optional<insurance_title>& dst,
	                      std::optional<insurance_title>&& opt,
	                      std::function<bool(std::string&&,
	                                         std::optional<insurance_title>&,
	                                         bool)> const& validator) {
		return get_field_answer_impl(ask_questions, label, dst, std::move(opt),
		                             validator);
	}

	static constexpr auto magic_currency = "minimal"sv;
	bool get_field_answer(bool ask_questions,
	                      std::string_view label,
	                      std::optional<currency>& dst,
	                      std::optional<currency>&& opt,
	                      std::function<bool(std::string&&,
	                                         std::optional<currency>&,
	                                         bool)> const& validator) {
		std::function<bool(std::string&&, std::optional<currency>&, bool)> const
		    wrapped = [validator](std::string&& in,
		                          std::optional<currency>& out,
		                          bool ask_questions) {
			    if (in.starts_with(magic_currency)) {
				    in = magic_currency;
			    }
			    return validator(std::move(in), out, ask_questions);
		    };
		return get_field_answer_impl(ask_questions, label, dst, std::move(opt),
		                             wrapped);
	}

	bool get_field_answer(
	    bool ask_questions,
	    std::string_view label,
	    std::optional<ratio>& dst,
	    std::optional<ratio>&& opt,
	    std::function<bool(std::string&&, std::optional<ratio>&, bool)> const&
	        validator) {
		return get_field_answer_impl(ask_questions, label, dst, std::move(opt),
		                             validator);
	}
}  // namespace quick_dra
