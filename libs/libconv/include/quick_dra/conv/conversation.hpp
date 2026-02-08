// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <args_parser.hpp>
#include <quick_dra/base/str.hpp>
#include <quick_dra/conv/concepts.hpp>
#include <quick_dra/conv/field_policy.hpp>
#include <quick_dra/conv/low_level.hpp>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace args {
	template <>
	struct converter<quick_dra::insurance_title> {
		static inline quick_dra::insurance_title
		value(parser& p, std::string const& arg, std::string const& name) {
			quick_dra::insurance_title out{};
			if (!quick_dra::insurance_title::parse(arg, out)) {
				p.error(fmt::format("{}: expecting 6 digits in form `#### # #'",
				                    name),
				        p.parse_width());
			}
			return out;
		}
	};

	template <>
	struct converter<quick_dra::ratio> {
		static inline quick_dra::ratio value(parser& p,
		                                     std::string const& arg,
		                                     std::string const& name) {
			quick_dra::ratio out{};
			if (!quick_dra::ratio::parse(arg, out) || out.den == 0) {
				p.error(fmt::format(
				            "{}: expecting two numbers in form `<num>/<den>`, "
				            "with denominator not equal to zero",
				            name),
				        p.parse_width());
			}
			return out;
		}
	};

	template <>
	struct converter<quick_dra::currency> {
		static inline quick_dra::currency value(parser& p,
		                                        std::string const& arg,
		                                        std::string const& name) {
			using std::literals::operator""sv;

			quick_dra::currency out{};
			if (arg == "minimal"sv || arg == "none"sv) {
				out = quick_dra::currency{-1 * quick_dra::currency::den};
			} else if (!quick_dra::currency::parse(arg, out)) {
				p.error(fmt::format("{}: expecting a number with 0.01 "
				                    "increment, with optional PLN or zł suffix",
				                    name),
				        p.parse_width());
			}
			return out;
		}
	};
}  // namespace args

namespace quick_dra {
	using std::literals::operator""s;

	struct arg_parser {
		args::null_translator tr{};
		args::parser parser;
		arg_parser(std::string_view tool_name,
		           args::arglist arguments,
		           std::string_view description)
		    : parser{as_str(description), {tool_name, arguments}, &tr} {}
	};

	template <typename Subject>
	struct conversation {
		using value_type = Subject;
		bool ask_questions{true};
		value_type opts{};
		value_type dst{};

		struct no_questions_verifier {
			conversation& conv;
			args::parser const& parser;
			template <FieldPolicyWithArgFlags... Policy>
			no_questions_verifier& required(Policy const&... policies) {
				conv.required_when_no_questions(parser, policies...);
				return *this;
			}
		};

		constexpr no_questions_verifier verifier(args::parser const& p) {
			return no_questions_verifier{*this, p};
		}

		template <typename Policy>
		bool get_answer(Policy const& policy) {
			return policy.get_answer(*this);
		}

		template <AnyFieldPolicy Policy>
		bool check_field(Policy const& policy) {
			return get_answer(policy.get_field_answer());
		}

		template <AnyFieldPolicy Policy, Enumerator... Items>
		bool check_enum_field(std::string_view switches_to_fix,
		                      Policy const& policy,
		                      Items&&... items) {
			auto const enum_field =
			    policy.get_enum_field(std::forward<Items>(items)...);
			if (!get_answer(enum_field)) {
				if (!this->ask_questions && !switches_to_fix.empty()) {
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

		template <typename Policy>
		bool show_added(Policy const& policy) {
			auto const& new_value = policy.select(dst);
			fmt::print("\033[0;90m{} set to \033[m{}\n", policy.label,
			           new_value
			               .transform([](auto const& value) {
				               return as_string(value);
			               })
			               .value_or("<empty>"s));
			return true;
		}

	private:
		template <FieldPolicyWithArgFlags... Policy>
		void required_when_no_questions(args::parser const& p,
		                                Policy const&... policies) {
			static constexpr auto const policies_count = sizeof...(Policy);

			if constexpr (policies_count == 1) {
				if (ask_questions) return;
				if (!this->check_required_on_no_questions(policies...)) {
					p.error(fmt::format("argument {} is required with -y",
					                    policies.arg_flag...));
				}
			} else if constexpr (policies_count > 1) {
				bool const present_in_cli[] = {
				    this->check_required_on_no_questions(
				        policies, check_side::command_line_arguments)...};
				std::string_view const arg_flags[] = {policies.arg_flag...};

				size_t cli_count = 0;
				for (auto const present : present_in_cli) {
					if (present) ++cli_count;
				}

				if (!ask_questions && !cli_count) {
					bool const present_in_config[] = {
					    this->check_required_on_no_questions(
					        policies, check_side::config_file)...};
					auto cfg_count = 0;
					for (auto const present : present_in_config) {
						if (present) ++cfg_count;
					}
					if (!cfg_count) {
						auto const all_but_last =
						    std::span{arg_flags}.subspan(0, policies_count - 1);
						auto const last_one = arg_flags[policies_count - 1];
						p.error(fmt::format(
						    "at least one of {} and {} is required with -y",
						    fmt::join(all_but_last, ", "), last_one));
					}
				}

				if (cli_count > 1) {
					std::vector<std::string_view> flags_present{};
					flags_present.reserve(cli_count);
					for (size_t index = 0; index < policies_count; ++index) {
						if (present_in_cli[index]) {
							flags_present.push_back(arg_flags[index]);
						}
					}

					auto const all_but_last = std::span{flags_present}.subspan(
					    0, flags_present.size() - 1);
					auto const last_one = flags_present.back();
					p.error(fmt::format(
					    "only one of {} and {} can be used at the same time",
					    fmt::join(all_but_last, ", "), last_one));
				}
			}
		}

		enum check_side {
			command_line_arguments = 1,
			config_file = 2,
			both = 3,
		};

		template <FieldPolicyWithArgFlags Policy>
		bool check_required_on_no_questions(
		    Policy const& policy,
		    check_side side = check_side::both) {
			auto result = false;
			if (side & check_side::command_line_arguments) {
				result |= !!policy.select(opts);
			}
			if (side & check_side::config_file) {
				result |= !!policy.select(dst);
			}
			return result;
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
	};
}  // namespace quick_dra
