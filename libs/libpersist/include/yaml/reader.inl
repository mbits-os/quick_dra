// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

namespace yaml {
	template <typename T>
	static inline bool read_key(ref_ctx const& ref,
	                            ryml::csubstr key,
	                            std::optional<T>& ctx) {
		auto const child = ref.ref().find_child(key);
		if (child.invalid()) {
			return true;
		}
		ctx.emplace();
		return read_value(ref.from(child), *ctx);
	}

	template <typename T>
	    requires(!is_optional<T>::value)
	static inline bool read_key(ref_ctx const& ref,
	                            ryml::csubstr key,
	                            T& ctx,
	                            bool optional) {
		auto const child = ref.ref().find_child(key);
		if (child.invalid()) {
			if (optional) return true;
			return ref.error(fmt::format("expecting `{}`", view(key)));
		}
		return read_value(ref.from(child), ctx);
	}

	template <typename T>
	bool is_valid(ref_ctx const& ref, std::vector<T> const&) {
		return ref.ref().is_seq();
	}

	template <typename T>
	bool is_valid(ref_ctx const& ref, std::set<T> const&) {
		return ref.ref().is_seq() || ref.ref().type().val_is_null();
	}

	template <typename K, typename T>
	bool is_valid(ref_ctx const& ref, std::map<K, T> const&) {
		return ref.ref().is_map() || ref.ref().type().val_is_null();
	}

	template <typename T>
	bool read_value_impl(ref_ctx const& ref, std::vector<T>& ctx) {
		if (!is_valid(ref, ctx)) {
			return ref.error("expecting an array"sv);
		}
		ctx.reserve(ctx.size() + ref.ref().num_children());
		for (auto const& child : ref.ref()) {
			ctx.emplace_back();
			if (!read_value(ref.from(child), ctx.back())) return false;
		}
		return true;
	}

	template <typename K, typename T>
	bool read_value_impl(ref_ctx const& ref, std::map<K, T>& ctx) {
		if (!is_valid(ref, ctx)) {
			return ref.error("expecting a map"sv);
		}

		for (auto const& child : ref.ref()) {
			auto const child_var = ref.from(child);

			if (!child.has_key()) {
				return child_var.error("expecting a map member");
			}

			K key{};
			if (!convert_string(child_var, child.key(), key)) return false;

			T& sub = ctx[std::move(key)];
			if (!read_value(child_var, sub)) return false;
		}
		return true;
	}

	template <typename T>
	bool read_value(ref_ctx const& ref, std::vector<T>& ctx) {
		return read_value_impl(ref, ctx);
	}

	template <typename K, typename T>
	bool read_value(ref_ctx const& ref, std::map<K, T>& ctx) {
		return read_value_impl(ref, ctx);
	}

	template <typename T>
	bool read_value(ref_ctx const& ref, std::set<T>& ctx) {
		if (!ref.ref().is_seq()) {
			return ref.error("expecting an array"sv);
		}

		for (auto const& child : ref.ref()) {
			T sub{};
			if (!read_value(ref.from(child), sub)) return false;
			ctx.insert(sub);
		}
		return true;
	}

	template <typename T>
	T read_reversed(ref_ctx const& ref, size_t index, bool& ok) {
		T ctx{};
		auto const& child = ref.ref()[index];
		auto const result = read_value(ref.from(child), ctx);
		if (!result) ok = false;
		return ctx;
	}

	template <typename... T, size_t... Index>
	bool read_tuple(ref_ctx const& ref,
	                std::tuple<T...>& ctx,
	                std::index_sequence<Index...>) {
		bool result = true;
		ctx = std::tuple{read_reversed<T>(ref, Index, result)...};
		return result;
	}

	template <typename... T>
	bool read_value(ref_ctx const& ref, std::tuple<T...>& ctx) {
		if (!ref.ref().is_seq()) {
			return ref.error("expecting an array"sv);
		}
		auto const size = ref.ref().num_children();
		if (size != sizeof...(T)) {
			return ref.error(
			    fmt::format("expecting an array of {} elements, got {}"sv,
			                sizeof...(T), size));
		}
		return read_tuple(ref, ctx, std::index_sequence_for<T...>{});
	}

	template <typename S, typename... T>
	bool read_single_value(ref_ctx const& ref, std::variant<T...>& ctx) {
		S value{};
		if (!read_value(ref, value)) {
			return false;
		}
		ctx = std::move(value);
		return true;
	}

	template <typename... T>
	bool read_value(ref_ctx const& ref, std::variant<T...>& ctx) {
		auto const result = (read_single_value<T>(ref, ctx) || ...);
		if (!result) {
			return ref.error("cannot match any union alternative"sv);
		}
		return true;
	}

	inline bool read_value(ref_ctx const& ref, std::integral auto& ctx) {
		if (!ref.ref().has_val()) {
			ctx = 0;
			return false;
		}

		auto const value = ref.val();
		return convert_string(ref, value, ctx);
	}

	inline bool convert_string(ref_ctx const& ref,
	                           auto const& value,
	                           std::integral auto& ctx) {
		if (value.empty()) {
			ctx = 0;
			return true;
		}

		auto const begin = value.data();
		auto const end = begin + value.size();
		auto const [ptr, ec] = std::from_chars(begin, end, ctx);
		if (ptr != end || ec != std::errc{}) {
			ctx = 0;
			if constexpr (std::is_signed_v<
			                  std::remove_cvref_t<decltype(ctx)>>) {
				return ref.error("expecting a number");
			} else {
				return ref.error("expecting a positive number");
			}
		}

		return true;
	}
}  // namespace yaml
