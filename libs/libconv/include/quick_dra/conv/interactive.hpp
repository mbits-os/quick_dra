// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <functional>
#include <quick_dra/conv/low_level.hpp>
#include <string>
#include <tuple>
#include <utility>

namespace quick_dra::interactive {
	template <typename Arg, FieldPolicy<Arg> FieldPolicy>
	struct field_answer : FieldPolicy {
		using value_type = typename FieldPolicy::value_type;
		using validator_fn = std::function<
		    bool(std::string&&, std::optional<value_type>&, bool)>;

		template <typename Conversation>
		bool get_answer(Conversation& conv) const {
			auto const& policy = *static_cast<FieldPolicy const*>(this);
			return get_field_answer(conv.ask_questions, policy.label,
			                        policy.select(conv.dst),
			                        std::move(policy.select(conv.opts)),
			                        validator_fn{policy.validator()});
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

	namespace details {
		template <typename T>
		struct is_enumerator_item : std::false_type {};

		template <typename T>
		struct is_enumerator_item<interactive::enumerator_item<T>>
		    : std::true_type {};

		template <typename T>
		concept Enumerator = static_cast<bool>(is_enumerator_item<T>{});
	};  // namespace details

	template <typename FieldPolicy, details::Enumerator... Items>
	struct enum_field : FieldPolicy {
		using tuple_type = std::tuple<Items...>;
		tuple_type items;

		enum_field(FieldPolicy const& policy, tuple_type const& items)
		    : FieldPolicy{policy}, items{items} {}

		template <typename Conversation>
		bool get_answer(Conversation& conv) const {
			auto const& policy = *static_cast<FieldPolicy const*>(this);
			auto tagger = [policy](auto& tgt) {
				auto const& ref = policy.select(tgt);
				return ref ? *ref : std::string_view{" ", 1};
			};

			conv.opts.postprocess_document_kind();
			auto selected = this->first_item_available(
			    tagger(conv.opts),
			    std::make_index_sequence<sizeof...(Items)>{});
			conv.opts.preprocess_document_kind();

			if (selected == '\0') {
				selected = this->first_item_available(
				    tagger(conv.dst),
				    std::make_index_sequence<sizeof...(Items)>{});
			}
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
		template <details::Enumerator EnumeratorItem>
		static bool item_available(char& code,
		                           std::string_view tag,
		                           EnumeratorItem const& item) {
			auto const item_set = tag.starts_with(item.code);
			if (item_set) {
				code = item.code;
			}
			return item_set;
		}

		template <size_t... Index>
		char first_item_available(
		    std::string_view tag,
		    std::index_sequence<Index...>) const noexcept {
			char code = 0;
			(item_available(code, tag, std::get<Index>(items)) || ...);
			return code;
		}

		template <details::Enumerator EnumeratorItem>
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
}  // namespace quick_dra::interactive

namespace quick_dra::concepts {
	using interactive::details::Enumerator;
}
