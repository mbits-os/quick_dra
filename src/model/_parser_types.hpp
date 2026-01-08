// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <fmt/format.h>
#include <fmt/std.h>
#include <charconv>
#include <concepts>
#include <filesystem>
#include <fstream>
#include <optional>
#include <quick_dra/model/types.hpp>
#include <ryml.hpp>
#include <ryml_std.hpp>
#include <set>
#include <string>
#include <variant>

using namespace std::literals;

namespace quick_dra {
	inline std::optional<std::string> open(std::filesystem::path const& path) {
		std::optional<std::string> result{};
		std::ifstream in{path, std::ios::in | std::ios::binary};
		if (!in) {
			fmt::print("quick_dra: error: cannot find {}\n", path);
			return result;
		}

		std::ostringstream contents;
		contents << in.rdbuf();
		result = std::move(contents).str();

		return result;
	}

	inline std::string_view view(ryml::csubstr const& sub) {
		if (sub.empty()) return {};
		return {sub.data(), sub.size()};
	}

	static bool parse_succeeded = true;

	inline bool error_handler(c4::yml::Location const& loc,
	                          std::string_view msg,
	                          std::string_view level = "error"sv) {
		if (!loc.name.empty()) {
			fmt::print(stderr, "{}:", view(loc.name));
		}
		fmt::print(stderr, "{}:{}: {}: {}\n", loc.line + 1, loc.col + 1, level,
		           msg);

		parse_succeeded = false;
		return false;
	}

	inline void diagnostic(file_var const& loc,
	                       std::string_view msg,
	                       std::string_view level = "error"sv) {
		if (!loc.filename.empty()) {
			fmt::print(stderr, "{}:", loc.filename);
		}
		fmt::print(stderr, "{}:{}: {}: {}\n", loc.line + 1, loc.col + 1, level,
		           msg);
	}

	inline void warn(file_var const& loc, std::string_view msg) {
		diagnostic(loc, msg, "warning"sv);
	}

	struct c4_error_exception {};

	inline void c4_error_handler(const char* msg,
	                             size_t msg_len,
	                             c4::yml::Location loc,
	                             void* /*user_data*/) {
		error_handler(loc, std::string_view{msg, msg_len});
		throw c4_error_exception{};
	}

	struct ref_ctx;

	struct base_ctx {
		ryml::Parser const* parser{nullptr};

		inline ref_ctx from(ryml::ConstNodeRef const& ref) const;
	};

	struct ref_ctx : base_ctx {
		ryml::ConstNodeRef const* ref_{};

		file_var get_location() const {
			if (!parser || !ref_) return {};
			auto const loc = ref_->location(*parser);
			return {
			    .filename{loc.name.data(), loc.name.size()},
			    .line = loc.line,
			    .col = loc.col,
			};
		};

		bool error(std::string_view const& msg) const {
			if (parser && ref_) {
				return error_handler(ref_->location(*parser), msg);
			}
			fmt::print(stderr, "error: {}\n", msg);

			parse_succeeded = false;
			return false;
		}

		void warn(std::string_view const& msg) const {
			if (parser && ref_) {
				error_handler(ref_->location(*parser), msg, "warning"sv);
				return;
			}
			fmt::print(stderr, "warning: {}\n", msg);
		}

		ryml::ConstNodeRef const& ref() const noexcept { return *ref_; }
		c4::csubstr val() const { return ref_ ? ref_->val() : c4::csubstr{}; }
	};

	inline ref_ctx base_ctx::from(ryml::ConstNodeRef const& ref) const {
		return {
		    {.parser = parser},
		    &ref,
		};
	}

	struct YAML {
		base_ctx context(base_ctx const& parent = {}) const {
			auto result = parent;
			result.parser = &parser;
			return result;
		}

		ryml::Parser build_parser() noexcept {
			ryml::Parser p{&evt_handler, ryml::ParserOptions{}.locations(true)};
			p.reserve_locations(300);
			return p;
		}

		ryml::EventHandlerTree evt_handler{};
		ryml::Parser parser{build_parser()};
		std::string contents{};
		std::string path_str{};

		std::optional<ryml::Tree> load(std::filesystem::path const& path) {
			auto maybe_contents = open(path);
			if (!maybe_contents) {
				return std::nullopt;
			}

			return load_contents(std::move(*maybe_contents), path.string());
		}

		template <typename StringLike>
		std::optional<ryml::Tree> load_contents(StringLike&& text,
		                                        std::string const& path) {
			contents = std::forward<StringLike>(text);
			path_str = path;
			parser.reserve_locations(300);
			auto tree = ryml::parse_in_place(
			    &parser, ryml::to_csubstr(path_str), ryml::to_substr(contents));
			tree.resolve();
			return std::optional{std::move(tree)};
		}
	};

	template <typename E>
	struct enum_tag {
		std::string_view id;
		E value;
	};

	template <typename T>
	struct is_optional : std::false_type {};
	template <typename T>
	struct is_optional<std::optional<T>> : std::true_type {};
}  // namespace quick_dra

namespace quick_dra::v1 {
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
	};

	template <ReadableValue T>
	bool read_value(ref_ctx const& ref, T& ctx) {
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

	template <typename T>
	bool read_value(ref_ctx const& ref, payload_with_location<T>& ctx);

	bool read_value(ref_ctx const& ref, bool& ctx);
	bool read_value(ref_ctx const& ref, std::integral auto& ctx);
	bool read_value(ref_ctx const& ref, percent& ctx);
	bool read_value(ref_ctx const& ref, currency& ctx);
	bool read_value(ref_ctx const& ref, ratio& ctx);
	bool read_value(ref_ctx const& ref, insurance_title& ctx);
	bool read_value(ref_ctx const& ref, std::string& ctx);

	bool convert_string(ref_ctx const& ref,
	                    c4::csubstr const& value,
	                    std::chrono::year_month& ctx);

	inline bool convert_string(ref_ctx const&,
	                           c4::csubstr const& value,
	                           std::string& ctx) {
		ctx = view(value);
		return true;
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
}  // namespace quick_dra::v1

#include "model/yaml_parser.hpp"

namespace quick_dra::v1 {
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
		return ref.ref().is_seq();
	}

	template <typename K, typename T>
	bool is_valid(ref_ctx const& ref, std::map<K, T> const&) {
		return ref.ref().is_map();
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
	bool read_value(ref_ctx const& ref, payload_with_location<T>& ctx) {
		if (!read_value(ref, ctx.payload)) return false;
		ctx.loc = ref.get_location();
		return true;
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
}  // namespace quick_dra::v1
