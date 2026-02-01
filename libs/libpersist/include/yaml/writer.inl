// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

namespace yaml {
	template <typename T>
	static inline void write_key(ryml::NodeRef& ref,
	                             ryml::csubstr key,
	                             std::optional<T> const& ctx) {
		if (!ctx) return;
		write_key(ref, key, *ctx);
	}

	template <typename T>
	    requires(!is_optional<T>::value)
	static inline void write_key(ryml::NodeRef& ref,
	                             ryml::csubstr key,
	                             T const& ctx) {
		auto child = ref.append_child();
		child << c4::yml::key(key);
		write_value(child, ctx);
	}

	template <typename T>
	void write_value(ryml::NodeRef& ref, std::vector<T> const& ctx) {
		ref |= c4::yml::SEQ;

		for (auto const& item : ctx) {
			auto child = ref.append_child();
			write_value(child, item);
		}
	}

	template <typename K, typename T>
	void write_value(ryml::NodeRef& ref, std::map<K, T> const& ctx) {
		ref |= c4::yml::MAP;

		for (auto const& [key, value] : ctx) {
			auto const& skey = as_string(key);
			write_key(ref, {skey.c_str(), skey.data()}, value);
		}
	}

	inline void write_value(ryml::NodeRef& ref, std::integral auto ctx) {
		ref << ctx;
	}
}  // namespace yaml
