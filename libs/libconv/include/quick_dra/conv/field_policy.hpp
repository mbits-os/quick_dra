// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <optional>
#include <quick_dra/base/chrono.hpp>
#include <quick_dra/base/types.hpp>
#include <quick_dra/conv/concepts.hpp>
#include <quick_dra/conv/interactive.hpp>
#include <quick_dra/models/yaml/model_versions.hpp>
#include <quick_dra/models/yaml/user_config_partial.hpp>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace quick_dra {
	namespace policy_builder {
		template <typename Arg>
		struct validator_function {
			bool (*validator)(std::string&&, std::optional<Arg>&, bool);
			bool operator()(std::string&& value, std::optional<Arg>& dst, bool ask_questions) const noexcept {
				return (*validator)(std::move(value), dst, ask_questions);
			}
		};

		static_assert(Validator<validator_function<int>, int>);
	}  // namespace policy_builder

	template <AnyFieldPolicy Policy>
	struct field_policy_with_arg_flags : Policy {
		std::string_view arg_flag;
	};

	template <typename Arg,
	          Selector<Arg> SelectorLambda,
	          Validator<Arg> ValidatorLambda = policy_builder::validator_function<Arg>>
	struct field_policy : SelectorLambda, ValidatorLambda {
		using selector_type = SelectorLambda;
		using validator_type = ValidatorLambda;
		using value_type = Arg;

		std::string_view label;

		using field_policy_with_arg_flags = quick_dra::field_policy_with_arg_flags<field_policy>;

		constexpr selector_type const& selector() const noexcept { return *this; }

		constexpr validator_type const& validator() const noexcept { return *this; }

		std::optional<value_type>& select(auto& payer) const noexcept { return this->selector()(payer); }

		std::optional<value_type> const& select(auto const& payer) const noexcept {
			return this->selector()(const_cast<std::remove_cvref_t<decltype(payer)>&>(payer));
		}

		constexpr Validator<value_type> auto get_validator() const noexcept {
			return [this](std::string&& value, std::optional<value_type>& dst, bool ask_questions) {
				return this->validator()(std::move(value), dst, ask_questions);
			};
		}

		constexpr interactive::field_answer<value_type, field_policy> get_field_answer() const noexcept {
			return {*this};
		}

		constexpr interactive::enumerator_item<field_policy> get_enum(char code) const noexcept {
			return {code, *this};
		}

		template <Enumerator... Items>
		constexpr interactive::enum_field<field_policy, Items...> get_enum_field(Items&&... items) const noexcept {
			return {*this, {std::forward<Items>(items)...}};
		}

		constexpr field_policy_with_arg_flags through(std::string_view arg_flag) const noexcept {
			return {*this, arg_flag};
		}
	};

	template <typename Arg,
	          SelectorWithLookup<Arg> SelectorLambda,
	          Validator<Arg> ValidatorLambda = policy_builder::validator_function<Arg>>
	struct lookup_field_policy : SelectorLambda, ValidatorLambda {
		using lookup_selector_type = SelectorLambda;
		using validator_type = ValidatorLambda;
		using value_type = Arg;

		std::string_view label;

		constexpr lookup_selector_type const& lookup_selector() const noexcept { return *this; }

		constexpr validator_type const& validator() const noexcept { return *this; }

		field_policy<value_type, typename lookup_selector_type::with_lookup, validator_type> operator[](
		    year_month const& month) const noexcept {
			return {this->lookup_selector()[month], this->validator(), this->label};
		}
	};

	template <EnumKeyEnabled FieldPolicy>
	inline auto get_enum_item(FieldPolicy const& policy) noexcept {
		return policy.get_enum(policy.enum_key);
	};

	namespace getters {
		struct first_name {
			using person_type = partial::person;
			inline std::optional<std::string>& operator()(person_type& person) const noexcept {
				return person.first_name;
			}
		};
		struct last_name {
			using person_type = partial::person;
			inline std::optional<std::string>& operator()(person_type& person) const noexcept {
				return person.last_name;
			}
		};
		struct social_id {
			using person_type = partial::insured_t;
			static constexpr char enum_key = 'P';
			inline std::optional<std::string>& operator()(auto& person) const noexcept { return person.social_id; }
		};
		struct id_card {
			using person_type = partial::person;
			static constexpr char enum_key = '1';
			inline std::optional<std::string>& operator()(person_type& payer) const noexcept { return payer.id_card; }
		};
		struct passport {
			using person_type = partial::person;
			static constexpr char enum_key = '2';
			inline std::optional<std::string>& operator()(person_type& person) const noexcept {
				return person.passport;
			}
		};
		struct kind {
			using person_type = partial::person;
			inline std::optional<std::string>& operator()(person_type& person) const noexcept { return person.kind; }
		};
		struct document {
			using person_type = partial::person;
			inline std::optional<std::string>& operator()(person_type& person) const noexcept {
				return person.document;
			}
		};
		struct tax_id {
			using person_type = partial::payer_t;
			inline std::optional<std::string>& operator()(person_type& payer) const noexcept { return payer.tax_id; }
		};

		struct title {
			using person_type = partial::insured_t;
			inline std::optional<insurance_title>& operator()(person_type& insured) const noexcept {
				return insured.title;
			}
		};

		struct history_t {
			year_month month{};
			history_t(year_month const& month) : month{month} {}
			partial::employment_history& get(partial::insured_t& insured) const {
				if (!insured.history) {
					insured.history.emplace();
				}
				return *find_or_add_to_timeline(month, *insured.history).second;
			}
		};

		struct part_time_scale {
			struct with_lookup : history_t {
				using person_type = partial::insured_t;
				inline std::optional<ratio>& operator()(person_type& insured) const {
					return get(insured).part_time_scale;
				}
			};
			with_lookup operator[](year_month const& month) const noexcept { return {month}; }
		};

		struct salary {
			struct with_lookup : history_t {
				using person_type = partial::insured_t;
				year_month month{};
				inline std::optional<currency>& operator()(person_type& insured) const { return get(insured).salary; }
			};
			with_lookup operator[](year_month const& month) const noexcept { return {month}; }
		};
	}  // namespace getters

	namespace policy_builder {
		namespace details {
			template <typename T>
			concept modifiable = std::same_as<T, std::remove_cvref_t<T>&>;

			template <typename T, typename Arg>
			concept selector_lambda_helper = requires(T const& lambda, Arg& arg) {
				{ lambda(arg) } -> modifiable;
			};

			template <typename T>
			concept selector_lambda = requires() {
				typename T::person_type;
				requires selector_lambda_helper<T, typename T::person_type>;
			};

			template <typename T>
			concept selector_lambda_with_lookup = requires(T const& lambda) {
				typename T::with_lookup;
				requires selector_lambda<typename T::with_lookup>;
				{ lambda[0y / 1] } -> std::convertible_to<typename T::with_lookup>;
			};

			template <selector_lambda T>
			using selector_lambda_return_type = decltype(std::declval<T>()(std::declval<typename T::person_type&>()));

			template <selector_lambda_with_lookup T>
			using selector_lambda_with_lookup_return_type = selector_lambda_return_type<typename T::with_lookup>;

			template <typename T>
			struct value_type_of {
				using type = T;
			};

			template <typename T>
			struct value_type_of<std::optional<T>> {
				using type = T;
			};

			template <selector_lambda T>
			struct value_type_of<T> : value_type_of<std::remove_cvref_t<selector_lambda_return_type<T>>> {};

			template <selector_lambda_with_lookup T>
			struct value_type_of<T> : value_type_of<std::remove_cvref_t<selector_lambda_with_lookup_return_type<T>>> {};

			template <typename T>
			using value_type_t = typename value_type_of<T>::type;
		}  // namespace details

		template <typename Arg, Selector<Arg> SelectorLambda>
		struct label_selector : SelectorLambda {
			using value_type = Arg;

			std::string_view value;

			constexpr SelectorLambda const& selector() const noexcept { return *this; }

			template <Validator<value_type> ValidatorLambda>
			[[nodiscard]] friend consteval field_policy<value_type, SelectorLambda, ValidatorLambda> operator/(
			    label_selector const& lhs,
			    ValidatorLambda&& validator) {
				return {lhs.selector(), std::forward<ValidatorLambda>(validator), lhs.value};
			}

			[[nodiscard]] friend consteval field_policy<value_type, SelectorLambda, validator_function<value_type>>
			operator/(label_selector const& lhs, bool (*validator)(std::string&&, std::optional<value_type>&, bool)) {
				return {lhs.selector(), {validator}, lhs.value};
			}
		};

		template <typename Arg, SelectorWithLookup<Arg> SelectorLambda>
		struct lookup_label_selector : SelectorLambda {
			using value_type = Arg;

			std::string_view value;

			constexpr SelectorLambda const& lookup_selector() const noexcept { return *this; }

			template <Validator<value_type> ValidatorLambda>
			[[nodiscard]] friend consteval lookup_field_policy<value_type, SelectorLambda, ValidatorLambda> operator/(
			    lookup_label_selector const& lhs,
			    ValidatorLambda&& validator) {
				return {lhs.lookup_selector(), std::forward<ValidatorLambda>(validator), lhs.value};
			}

			[[nodiscard]] friend consteval lookup_field_policy<value_type,
			                                                   SelectorLambda,
			                                                   validator_function<value_type>>
			operator/(lookup_label_selector const& lhs,
			          bool (*validator)(std::string&&, std::optional<value_type>&, bool)) {
				return {lhs.lookup_selector(), {validator}, lhs.value};
			}
		};

		struct label {
			std::string_view value;

			constexpr auto operator<=>(label const&) const noexcept = default;

			template <details::selector_lambda SelectorLambda>
			[[nodiscard]] consteval label_selector<details::value_type_t<SelectorLambda>, SelectorLambda> operator/(
			    SelectorLambda const& val) const noexcept {
				return {val, value};
			}

			template <details::selector_lambda_with_lookup SelectorLambda>
			[[nodiscard]] consteval lookup_label_selector<details::value_type_t<SelectorLambda>, SelectorLambda>
			operator/(SelectorLambda const& val) const noexcept {
				return {val, value};
			}
		};
	}  // namespace policy_builder

	[[nodiscard]] inline consteval policy_builder::label operator""_label(char const* str, size_t length) {
		return {.value{str, length}};
	}

	[[nodiscard]] inline consteval policy_builder::label label(std::string_view value) { return {.value{value}}; }
}  // namespace quick_dra
