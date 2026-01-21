// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <charconv>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <variant>
#include <vector>
#include <yaml/ref.hpp>

namespace yaml {
	using std::literals::operator""sv;

	template <typename E>
	struct enum_tag {
		std::string_view id;
		E value;
	};

	template <typename T>
	struct is_optional : std::false_type {};
	template <typename T>
	struct is_optional<std::optional<T>> : std::true_type {};

	template <typename T>
	static inline bool read_local_or_external(ref_ctx const& ref, T& ctx);

	template <typename T>
	static inline bool read_key(ref_ctx const& ref,
	                            ryml::csubstr key,
	                            std::optional<T>& ctx);

	template <typename T>
	    requires(!is_optional<T>::value)
	static inline bool read_key(ref_ctx const& ref,
	                            ryml::csubstr key,
	                            T& ctx,
	                            bool optional = false);

	template <typename T>
	bool read_value(ref_ctx const& ref, std::vector<T>& ctx);

	template <typename T>
	bool read_value(ref_ctx const& ref, std::set<T>& ctx);

	template <typename... T>
	bool read_value(ref_ctx const& ref, std::tuple<T...>& ctx);

	template <typename K, typename T>
	bool read_value(ref_ctx const& ref, std::map<K, T>& ctx);

	template <typename... T>
	bool read_value(ref_ctx const& ref, std::variant<T...>& ctx);

	template <typename T>
	concept ReadableValue = requires(ref_ctx const& ref, T& ctx) {
		{ ctx.read(ref) } -> std::same_as<bool>;
	};  // NOLINT(readability/braces)

	template <ReadableValue T>
	inline bool read_value(ref_ctx const& ref, T& ctx) {
		return ctx.read(ref);
	}

	template <typename E>
	    requires std::is_enum_v<E>
	bool read_value(ref_ctx const& ref, E& ctx) {
		auto const sub = ref.val();
		auto const value = view(sub);

		auto const items = enums(ctx);
		auto const item =
		    std::find_if(std::begin(items), std::end(items),
		                 [=](auto const& pair) { return pair.id == value; });
		if (item != std::end(items)) {
			ctx = item->value;
			return true;
		}

		return ref.error(fmt::format("unknown value, `{}`", value));
	}

	bool read_value(ref_ctx const& ref, bool& ctx);
	bool read_value(ref_ctx const& ref, std::integral auto& ctx);
	bool read_value(ref_ctx const& ref, std::string& ctx);

	bool convert_string(ref_ctx const&,
	                    c4::csubstr const& value,
	                    std::string& ctx);

	inline bool convert_string(ref_ctx const& ref,
	                           auto const& value,
	                           std::integral auto& ctx);
}  // namespace yaml

#include "reader.inl"
