// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <array>
#include <optional>
#include <quick_dra/conv/low_level.hpp>
#include <string>
#include <utility>

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

	template <typename FieldPolicy>
	struct enumerator_item : FieldPolicy {
		char code;

		enumerator_item(char code, FieldPolicy const& policy)
		    : FieldPolicy{policy}, code{code} {}

		constexpr std::pair<char, std::string_view> get_item() const noexcept {
			auto const& policy = *static_cast<FieldPolicy const*>(this);
			return {this->code, policy.label};
		}
	};

	template <typename T>
	struct is_enumerator_item : std::false_type {};

	template <typename T>
	struct is_enumerator_item<enumerator_item<T>> : std::true_type {};

	template <typename T>
	concept enumerator = static_cast<bool>(is_enumerator_item<T>{});

	template <typename FieldPolicy, enumerator... Items>
	struct enum_field : FieldPolicy {
		using tuple_type = std::tuple<Items...>;
		tuple_type items;

		enum_field(FieldPolicy const& policy, tuple_type const& items)
		    : FieldPolicy{policy}, items{items} {}

		template <typename Conversation>
		bool get_answer(Conversation& conv) const {
			auto const& policy = *static_cast<FieldPolicy const*>(this);
			auto selected = this->first_item_available(
			    conv, std::make_index_sequence<sizeof...(Items)>{});
			auto& dst = policy.select(conv.dst);
			if (conv.ask_questions) {
				return this->build_enum_answer(
				    dst, selected,
				    std::make_index_sequence<sizeof...(Items)>{});
			}
			if (!selected) {
				dst.reset();
				return false;
			}

			dst = std::string{selected};
			return true;
		}

	private:
		template <typename Conversation, enumerator EnumeratorItem>
		static bool item_available(char& code,
		                           Conversation& conv,
		                           EnumeratorItem const& item) {
			auto const item_set = !!item.select(conv.opts);
			if (item_set) {
				code = item.code;
			}
			return item_set;
		}

		template <typename Conversation, size_t... Index>
		char first_item_available(
		    Conversation& conv,
		    std::index_sequence<Index...>) const noexcept {
			char code = 0;
			(item_available(code, conv, std::get<Index>(items)) || ...);
			return code;
		}

		template <enumerator EnumeratorItem>
		static auto enum_label(EnumeratorItem const& item) {
			return std::pair{item.code, item.label};
		}

		template <size_t... Index>
		bool build_enum_answer(std::optional<std::string>& dst,
		                       char selected,
		                       std::index_sequence<Index...>) const {
			auto const& policy = *static_cast<FieldPolicy const*>(this);
			auto const labels =
			    std::array{enum_label(std::get<Index>(items))...};
			return get_enum_answer(
			    policy.label, labels, [&](char key) { dst = std::string{key}; },
			    selected);
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

		constexpr enumerator_item<field_policy> get_enum(
		    char code) const noexcept {
			return {code, *this};
		}

		template <enumerator... Items>
		constexpr enum_field<field_policy, Items...> get_enum_field(
		    Items&&... items) const noexcept {
			return {*this, {std::forward<Items>(items)...}};
		}
	};

	template <typename T>
	concept enum_key_enabled = requires() {
		{ T::enum_key } -> std::convertible_to<char>;
	};

	template <enum_key_enabled FieldPolicy>
	inline auto get_enum_item(FieldPolicy const& policy) noexcept {
		return policy.get_enum(policy.enum_key);
	};

	namespace getters {
		struct first_name {
			template <typename Person>
			inline std::optional<std::string>& operator()(
			    Person& person) const noexcept {
				return person.first_name;
			}
		};
		struct last_name {
			template <typename Person>
			inline std::optional<std::string>& operator()(
			    Person& person) const noexcept {
				return person.last_name;
			}
		};
		struct tax_id {
			template <typename Payer>
			inline std::optional<std::string>& operator()(
			    Payer& payer) const noexcept {
				return payer.tax_id;
			}
		};
		struct social_id {
			static constexpr char enum_key = 'P';
			template <typename Payer>
			inline std::optional<std::string>& operator()(
			    Payer& payer) const noexcept {
				return payer.social_id;
			}
		};
		struct id_card {
			static constexpr char enum_key = '1';
			template <typename Payer>
			inline std::optional<std::string>& operator()(
			    Payer& payer) const noexcept {
				return payer.id_card;
			}
		};
		struct passport {
			static constexpr char enum_key = '2';
			template <typename Payer>
			inline std::optional<std::string>& operator()(
			    Payer& payer) const noexcept {
				return payer.passport;
			}
		};
		struct kind {
			template <typename Payer>
			inline std::optional<std::string>& operator()(
			    Payer& payer) const noexcept {
				return payer.kind;
			}
		};
		struct document {
			template <typename Payer>
			inline std::optional<std::string>& operator()(
			    Payer& payer) const noexcept {
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
			[[nodiscard]] friend consteval field_policy<SelectorLambda,
			                                            ValidatorLambda>
			operator/(label_selector<SelectorLambda> const& lhs,
			          ValidatorLambda&& validator) {
				return {lhs.selector(),
				        std::forward<ValidatorLambda>(validator), lhs.value};
			}

			[[nodiscard]] friend consteval field_policy<SelectorLambda,
			                                            validator_function>
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
			[[nodiscard]] consteval label_selector<SelectorLambda> with()
			    const noexcept {
				return {SelectorLambda{}, value};
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
