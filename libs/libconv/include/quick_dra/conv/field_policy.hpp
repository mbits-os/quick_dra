// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <optional>
#include <string>

namespace quick_dra {
	template <typename FieldPolicy>
	struct string_field : FieldPolicy {
		template <typename Conversation>
		bool get_answer(Conversation& conv) const {
			auto const& policy = *static_cast<FieldPolicy const*>(this);
			return get_string_answer(
			    conv.ask_questions, policy.label, policy.select(conv.dst),
			    std::move(policy.select(conv.opts)), policy.get_validator());
		}
	};

	template <typename SelectorLambda, typename ValidatorLambda>
	struct field_policy : SelectorLambda, ValidatorLambda {
		std::string_view label;

		constexpr SelectorLambda const& selector() const noexcept {
			return *this;
		}

		constexpr ValidatorLambda const& validator() const noexcept {
			return *this;
		}

		std::optional<std::string>& select(auto& payer) const noexcept {
			return this->selector()(payer);
		}

		std::optional<std::string> const& select(
		    auto const& payer) const noexcept {
			return this->selector()(
			    const_cast<std::remove_cvref_t<decltype(payer)>&>(payer));
		}

		constexpr auto get_validator() const noexcept {
			return [this](std::string&& value, std::optional<std::string>& dst,
			              bool ask_questions) {
				return this->validator()(std::move(value), dst, ask_questions);
			};
		}

		constexpr string_field<field_policy> get_string_field() const noexcept {
			return {*this};
		}
	};

	namespace getters {
		struct first_name {
			inline std::optional<std::string>& operator()(
			    auto& payer) const noexcept {
				return payer.first_name;
			}
		};
		struct last_name {
			inline std::optional<std::string>& operator()(
			    auto& payer) const noexcept {
				return payer.last_name;
			}
		};
		struct tax_id {
			inline std::optional<std::string>& operator()(
			    auto& payer) const noexcept {
				return payer.tax_id;
			}
		};
		struct social_id {
			inline std::optional<std::string>& operator()(
			    auto& payer) const noexcept {
				return payer.social_id;
			}
		};
		struct id_card {
			inline std::optional<std::string>& operator()(
			    auto& payer) const noexcept {
				return payer.id_card;
			}
		};
		struct passport {
			inline std::optional<std::string>& operator()(
			    auto& payer) const noexcept {
				return payer.passport;
			}
		};
		struct kind {
			inline std::optional<std::string>& operator()(
			    auto& payer) const noexcept {
				return payer.kind;
			}
		};
		struct document {
			inline std::optional<std::string>& operator()(
			    auto& payer) const noexcept {
				return payer.document;
			}
		};
	}  // namespace getters

	namespace policy_builder {
		struct validator_function {
			bool (*validator)(std::string&&, std::optional<std::string>&, bool);
			bool operator()(std::string&& value,
			                std::optional<std::string>& dst,
			                bool ask_questions) const noexcept {
				return (*validator)(std::move(value), dst, ask_questions);
			}
		};

		template <typename SelectorLambda>
		struct label_selector : SelectorLambda {
			std::string_view value;

			constexpr SelectorLambda const& selector() const noexcept {
				return *this;
			}

			template <typename ValidatorLambda>
			friend consteval
			    [[nodiscard]] field_policy<SelectorLambda, ValidatorLambda>
			    operator/(label_selector<SelectorLambda> const& lhs,
			              ValidatorLambda&& validator) {
				return {lhs.selector(),
				        std::forward<ValidatorLambda>(validator), lhs.value};
			}

			friend consteval
			    [[nodiscard]] field_policy<SelectorLambda, validator_function>
			    operator/(label_selector<SelectorLambda> const& lhs,
			              bool (*validator)(std::string&& value,
			                                std::optional<std::string>&,
			                                bool)) {
				return {lhs.selector(), validator, lhs.value};
			}
		};

		struct label {
			std::string_view value;

			template <typename SelectorLambda>
			consteval [[nodiscard]] label_selector<SelectorLambda> with()
			    const noexcept {
				return {.value = value};
			}
		};
	}  // namespace policy_builder

	inline consteval [[nodiscard]] policy_builder::label operator""_label(
	    char const* str,
	    size_t length) {
		return {.value{str, length}};
	}

	[[nodiscard]] inline consteval policy_builder::label label(
	    std::string_view value) {
		return {.value{value}};
	}
}  // namespace quick_dra
