// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
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
	    std::function<void(char)> const& store_enum) {
		std::string hint{};
		hint.reserve([&items]() {
			size_t hint_size = items.size() * 4 + (items.size() - 1) * 2;
			for (auto const& [_, description] : items) {
				hint_size += description.size();
			}
			return hint_size;
		}());

		for (auto const& [value, description] : items) {
			if (!hint.empty()) hint.append(", "sv);
			hint.push_back(value);
			hint.append(" - "sv);
			hint.append(description);
		}

		return get_answer(label, hint, [&](std::string&& answer) {
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

	bool get_string_answer(bool ask_questions,
	                       std::string_view label,
	                       std::optional<std::string>& dst,
	                       std::optional<std::string>&& opt,
	                       std::function<bool(std::string&&,
	                                          std::optional<std::string>&,
	                                          bool)> const& validator) {
		if (opt) {
			dst = std::move(opt);
		}

		if (!ask_questions) {
			if (dst) {
				auto copy = *dst;
				auto const is_valid = validator(std::move(copy), dst, false);
				if (!is_valid) {
					comment("Cannot save invalid data with -y. Stopping.");
					return false;
				}
			}
			return true;
		}

		auto def_value = dst;
		return get_answer(label, def_value.value_or(""s),
		                  [&](std::string&& answer) {
			                  if (answer.empty() && def_value) {
				                  answer = *def_value;
			                  }
			                  return validator(std::move(answer), dst, true);
		                  });
	}
}  // namespace quick_dra
