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
#include <yaml/reader.hpp>

namespace yaml {
	template <typename T>
	static inline void write_key(ryml::NodeRef& ref,
	                             ryml::csubstr key,
	                             std::optional<T> const& ctx);

	template <typename T>
	    requires(!is_optional<T>::value)
	static inline void write_key(ryml::NodeRef& ref,
	                             ryml::csubstr key,
	                             T const& ctx);

	template <typename T>
	void write_value(ryml::NodeRef& ref, std::vector<T> const& ctx);

	template <typename K, typename T>
	void write_value(ryml::NodeRef& ref, std::map<K, T> const& ctx);

	template <typename T>
	concept WriteableValue = requires(ryml::NodeRef& ref, T const& ctx) {
		{ ctx.write(ref) };
	};  // NOLINT(readability/braces)

	template <WriteableValue T>
	inline void write_value(ryml::NodeRef& ref, T const& ctx) {
		ref |= c4::yml::MAP;
		ctx.write(ref);
	}

	void write_value(ryml::NodeRef& ref, bool ctx);
	void write_value(ryml::NodeRef& ref, std::integral auto ctx);
	void write_value(ryml::NodeRef& ref, std::string const& ctx);

	template <typename T>
	concept PreparableValue = requires(T& ctx) {
		{ ctx.prepare_for_write() };
	};  // NOLINT(readability/braces)

	void prepare_child(auto&) {}
	void prepare_child(PreparableValue auto& child) {
		child.prepare_for_write();
	}
	template <PreparableValue T>
	void prepare_child(std::vector<T>& child) {
		for (auto& item : child)
			item.prepare_for_write();
	}
	template <typename K, PreparableValue T>
	void prepare_child(std::map<K, T>& child) {
		for (auto& [_, item] : child)
			item.prepare_for_write();
	}
	template <typename T>
	void prepare_child(std::optional<T>& child) {
		if (child) {
			prepare_child(*child);
		}
	}
}  // namespace yaml

#include "writer.inl"
