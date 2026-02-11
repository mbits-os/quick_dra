// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <optional>
#include <quick_dra/base/types.hpp>
#include <quick_dra/conv/concepts.hpp>
#include <quick_dra/conv/interactive.hpp>
#include <quick_dra/models/yaml/user_config_partial.hpp>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace quick_dra {
	template <AnyFieldPolicy Policy>
	struct field_policy_with_arg_flags : Policy {
		std::string_view arg_flag;
	};

	template <typename Arg,
	          Selector<Arg> SelectorLambda,
	          Validator<Arg> ValidatorLambda>
	struct field_policy : SelectorLambda, ValidatorLambda {
		using selector_type = SelectorLambda;
		using validator_type = ValidatorLambda;
		using value_type = Arg;

		std::string_view label;

		using field_policy_with_arg_flags =
		    quick_dra::field_policy_with_arg_flags<field_policy>;

		constexpr selector_type const& selector() const noexcept {
			return *this;
		}

		constexpr validator_type const& validator() const noexcept {
			return *this;
		}

		std::optional<value_type>& select(auto& payer) const noexcept {
			return this->selector()(payer);
		}

		std::optional<value_type> const& select(
		    auto const& payer) const noexcept {
			return this->selector()(
			    const_cast<std::remove_cvref_t<decltype(payer)>&>(payer));
		}

		constexpr Validator<value_type> auto get_validator() const noexcept {
			return [this](std::string&& value, std::optional<value_type>& dst,
			              bool ask_questions) {
				return this->validator()(std::move(value), dst, ask_questions);
			};
		}

		constexpr interactive::field_answer<value_type, field_policy>
		get_field_answer() const noexcept {
			return {*this};
		}

		constexpr interactive::enumerator_item<field_policy> get_enum(
		    char code) const noexcept {
			return {code, *this};
		}

		template <Enumerator... Items>
		constexpr interactive::enum_field<field_policy, Items...>
		get_enum_field(Items&&... items) const noexcept {
			return {*this, {std::forward<Items>(items)...}};
		}

		constexpr field_policy_with_arg_flags through(
		    std::string_view arg_flag) const noexcept {
			return {*this, arg_flag};
		}
	};

	template <EnumKeyEnabled FieldPolicy>
	inline auto get_enum_item(FieldPolicy const& policy) noexcept {
		return policy.get_enum(policy.enum_key);
	};

	namespace getters {
		struct first_name {
			using person_type = partial::person;
			inline std::optional<std::string>& operator()(
			    person_type& person) const noexcept {
				return person.first_name;
			}
		};
		struct last_name {
			using person_type = partial::person;
			inline std::optional<std::string>& operator()(
			    person_type& person) const noexcept {
				return person.last_name;
			}
		};
		struct social_id {
			using person_type = partial::insured_t;
			static constexpr char enum_key = 'P';
			inline std::optional<std::string>& operator()(
			    auto& person) const noexcept {
				return person.social_id;
			}
		};
		struct id_card {
			using person_type = partial::person;
			static constexpr char enum_key = '1';
			inline std::optional<std::string>& operator()(
			    person_type& payer) const noexcept {
				return payer.id_card;
			}
		};
		struct passport {
			using person_type = partial::person;
			static constexpr char enum_key = '2';
			inline std::optional<std::string>& operator()(
			    person_type& person) const noexcept {
				return person.passport;
			}
		};
		struct kind {
			using person_type = partial::person;
			inline std::optional<std::string>& operator()(
			    person_type& person) const noexcept {
				return person.kind;
			}
		};
		struct document {
			using person_type = partial::person;
			inline std::optional<std::string>& operator()(
			    person_type& person) const noexcept {
				return person.document;
			}
		};
		struct tax_id {
			using person_type = partial::payer_t;
			inline std::optional<std::string>& operator()(
			    person_type& payer) const noexcept {
				return payer.tax_id;
			}
		};

		struct title {
			using person_type = partial::insured_t;
			inline std::optional<insurance_title>& operator()(
			    person_type& insured) const noexcept {
				return insured.title;
			}
		};

		struct part_time_scale {
			using person_type = partial::insured_t;
			inline std::optional<ratio>& operator()(
			    person_type& insured) const noexcept {
				return insured.part_time_scale;
			}
		};

		struct salary {
			using person_type = partial::insured_t;
			inline std::optional<currency>& operator()(
			    person_type& insured) const noexcept {
				return insured.salary;
			}
		};
	}  // namespace getters

	namespace policy_builder {
		namespace details {
			template <typename T>
			struct value_type_of {
				using type = T;
			};

			template <typename T>
			struct value_type_of<std::optional<T>> {
				using type = T;
			};

			template <typename T>
			    requires requires() { typename T::person_type; }
			struct value_type_of<T>
			    : value_type_of<std::remove_cvref_t<decltype(std::declval<T>()(
			          std::declval<typename T::person_type&>()))>> {};

			template <typename T>
			using value_type_t = typename value_type_of<T>::type;
		}  // namespace details

		template <typename Arg>
		struct validator_function {
			bool (*validator)(std::string&&, std::optional<Arg>&, bool);
			bool operator()(std::string&& value,
			                std::optional<Arg>& dst,
			                bool ask_questions) const noexcept {
				return (*validator)(std::move(value), dst, ask_questions);
			}
		};

		static_assert(Validator<validator_function<int>, int>);

		template <typename Arg, Selector<Arg> SelectorLambda>
		struct label_selector : SelectorLambda {
			using value_type = Arg;

			std::string_view value;

			constexpr SelectorLambda const& selector() const noexcept {
				return *this;
			}

			template <Validator<value_type> ValidatorLambda>
			[[nodiscard]] friend consteval field_policy<value_type,
			                                            SelectorLambda,
			                                            ValidatorLambda>
			operator/(label_selector const& lhs, ValidatorLambda&& validator) {
				return {lhs.selector(),
				        std::forward<ValidatorLambda>(validator), lhs.value};
			}

			[[nodiscard]] friend consteval field_policy<
			    value_type,
			    SelectorLambda,
			    validator_function<value_type>>
			operator/(label_selector const& lhs,
			          bool (*validator)(std::string&&,
			                            std::optional<value_type>&,
			                            bool)) {
				return {lhs.selector(), {validator}, lhs.value};
			}
		};

		struct label {
			std::string_view value;

			template <typename SelectorLambda>
			[[nodiscard]] consteval label_selector<
			    details::value_type_t<SelectorLambda>,
			    SelectorLambda>
			operator/(SelectorLambda const& val) const noexcept {
				return {val, value};
			}
		};
	}  // namespace policy_builder

	[[nodiscard]] inline consteval policy_builder::label operator""_label(
	    char const* str,
	    size_t length) {
		return {.value{str, length}};
	}

	[[nodiscard]] inline consteval policy_builder::label label(
	    std::string_view value) {
		return {.value{value}};
	}
}  // namespace quick_dra
