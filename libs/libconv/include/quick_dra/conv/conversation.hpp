// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <fmt/printf.h>
#include <quick_dra/conv/field_policy.hpp>
#include <string>

namespace quick_dra {
	using std::literals::operator""s;

	template <typename Subject>
	struct conversation {
		using value_type = Subject;
		bool ask_questions{true};
		value_type opts{};
		value_type dst{};

		template <typename Policy>
		bool get_answer(Policy const& policy) {
			return policy.get_answer(*this);
		}

		template <typename Selector, typename Validator>
		bool check_string_field(
		    field_policy<Selector, Validator> const& policy) {
			return get_answer(policy.get_string_field());
		}

		template <typename Policy>
		bool show_modified(Policy const& policy, value_type const& orig) {
			auto const& new_value = policy.select(dst);
			auto const& orig_value = policy.select(orig);
			if (new_value == orig_value) return false;
			fmt::print(
			    "\033[0;90m{} changed from \033[m{}\033[0;90m to "
			    "\033[m{}\n",
			    policy.label, orig_value.value_or("<empty>"s),
			    new_value.value_or("<empty>"s));
			return true;
		}
	};
}  // namespace quick_dra
