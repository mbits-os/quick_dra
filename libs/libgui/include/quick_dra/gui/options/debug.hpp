// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <concepts>
#include <map>
#include <optional>
#include <quick_dra/gui/options/dbg_types.hpp>
#include <string>
#include <string_view>
#include <utility>

using namespace std::literals;

#if defined GUI_DEBUG_ENABLED && GUI_DEBUG_ENABLED == 1
#define GUI_DEBUG
#endif

#if defined GUI_DEBUG_FRAMELESS_ENABLED && GUI_DEBUG_FRAMELESS_ENABLED == 1
#define GUI_DEBUG_FRAMELESS
#endif

namespace quick_dra {
	template <typename T>
	struct type_or_view {
		using type = T;
	};

	template <>
	struct type_or_view<std::string> {
		using type = std::string_view;
	};

	template <typename Storage>
	struct debug_map {
	public:
		using opt_type = type_or_view<Storage>::type;

		void set(std::string_view key, Storage const& value) { props_[key] = value; }

		std::optional<opt_type> get(std::string_view key) const noexcept {
			auto it = props_.find(key);
			if (it == props_.end()) return std::nullopt;
			return it->second;
		}

		auto begin() const noexcept { return props_.begin(); }
		auto end() const noexcept { return props_.end(); }

	private:
		std::map<std::string_view, Storage> props_{};
	};

	struct debug_flag_map : debug_map<bool> {
		std::optional<bool> get_maybe(std::string_view key) const noexcept { return debug_map<bool>::get(key); }
		void set_defaults(std::map<std::string_view, bool>&& defaults) { defaults_ = std::move(defaults); }
		std::optional<bool> get_default(std::string_view key) const noexcept;
		bool get(std::string_view key) const noexcept {
			return debug_map<bool>::get(key)
			    .or_else([key, &self = *this]() { return self.get_default(key); })
			    .value_or(false);
		}

	private:
		std::map<std::string_view, bool> defaults_{};
	};

	struct debug_options {
		debug_map<std::string> options{};
		debug_flag_map flags{};

		void postproc(debug_suite const& suite);

	private:
		void handle_implies(comma_separated_string_view dbg_implies, debug_suite const& suite);
	};
};  // namespace quick_dra
