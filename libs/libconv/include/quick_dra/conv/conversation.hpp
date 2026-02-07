// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <fmt/format.h>
#include <quick_dra/conv/concepts.hpp>
#include <quick_dra/conv/field_policy.hpp>
#include <quick_dra/conv/low_level.hpp>
#include <string_view>
#include <tuple>
#include <utility>

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

		template <typename Arg,
		          Selector<Arg> Selector,
		          Validator<Arg> Validator>
		bool check_field(field_policy<Arg, Selector, Validator> const& policy) {
			return get_answer(policy.get_field_answer());
		}

		template <typename Arg,
		          Selector<Arg> Selector,
		          Validator<Arg> Validator,
		          Enumerator... Items>
		bool check_enum_field(
		    std::string_view switches_to_fix,
		    field_policy<Arg, Selector, Validator> const& policy,
		    Items&&... items) {
			auto const enum_field =
			    policy.get_enum_field(std::forward<Items>(items)...);
			if (!get_answer(enum_field)) {
				if (!this->ask_questions) {
					comment(fmt::format(
					    "Cannot guess document kind based on information "
					    "given. Please use either {} with -y",
					    switches_to_fix));
				}
				return false;
			}
			auto const key = policy.select(this->dst);
			this->dst.preprocess_document_kind();
			if (!this->continue_with_enums(
			        key->front(), enum_field.items,
			        std::make_index_sequence<sizeof...(Items)>{})) {
				return false;
			}
			this->dst.postprocess_document_kind();
			return true;
		}

		template <Enumerator... Items, size_t... Index>
		bool continue_with_enums(char code,
		                         std::tuple<Items...> const& items,
		                         std::index_sequence<Index...>) {
			auto const success =
			    (this->continue_with_enum_item(code, std::get<Index>(items)) &&
			     ...);
			if (!success) return false;
			(this->clean_unmatched_fields(code, std::get<Index>(items)), ...);
			return true;
		}

		template <Enumerator Item>
		bool continue_with_enum_item(char code, Item const& policy) {
			if (code != policy.code) {
				return true;
			}

			return this->check_field(policy);
		}

		template <Enumerator Item>
		void clean_unmatched_fields(char code, Item const& policy) {
			if (code != policy.code) {
				policy.select(this->dst).reset();
			}
		}

		template <typename Policy>
		bool show_modified(Policy const& policy, value_type const& orig) {
			auto const& new_value = policy.select(dst);
			auto const& orig_value = policy.select(orig);
			if (new_value == orig_value) return false;
			fmt::print(
			    "\033[0;90m{} changed from \033[m{}\033[0;90m to \033[m{}\n",
			    policy.label, orig_value.value_or("<empty>"s),
			    new_value.value_or("<empty>"s));
			return true;
		}
	};
}  // namespace quick_dra
