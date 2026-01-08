// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <map>
#include <quick_dra/base/meta.hpp>
#include <quick_dra/base/str.hpp>
#include <quick_dra/model/base_types.hpp>
#include <quick_dra/model/model.hpp>
#include <quick_dra/model/yaml.hpp>
#include <string>
#include <string_view>
#include <variant>

namespace quick_dra {
	struct global_object;

	struct file_var {
		std::string filename{};
		size_t line{};
		size_t col{};
	};

	template <typename Payload>
	struct payload_with_location {
		using payload_type = Payload;
		Payload payload{};
		file_var loc{};
	};

	template <typename T>
	struct stored_type {
		using type = T;
	};
	template <>
	struct stored_type<std::string_view> {
		using type = std::string;
	};

	struct global_object {
		std::optional<maybe_list<calculated_value>> value{};

		std::map<std::string, global_object> children{};

		void insert(varname const& var, maybe_list<calculated_value>&& data) {
			get(var).value = std::move(data);
		}

		void insert(varname const& var, contribution const& data) {
			get(var) = data;
		}

		global_object& get(varname const& var) {
			auto ptr = this;

			for (auto const& path : var.path) {
				auto it = ptr->children.lower_bound(path);
				if (it == ptr->children.end() || it->first != path) {
					it = ptr->children.insert(it, {path, global_object{}});
				}
				ptr = &it->second;
			}

			return *ptr;
		}

		global_object const* peek(varname const& var) const {
			auto ptr = this;

			for (auto const& path : var.path) {
				auto it = ptr->children.find(path);
				if (it == ptr->children.end()) {
					return nullptr;
				}
				ptr = &it->second;
			}

			return ptr;
		}

		maybe_list<calculated_value> value_or(
		    varname const& var,
		    maybe_list<calculated_value> const& default_value) const noexcept {
			auto ptr = this;

			for (auto const& path : var.path) {
				auto it = ptr->children.find(path);
				if (it == ptr->children.end()) {
					return default_value;
				}
				ptr = &it->second;
			}

			return ptr->value.value_or(default_value);
		}

		template <typename T>
		T typed_value(varname const& var,
		              T const& default_value = {}) const noexcept {
			auto const val = value_or(var, {});

			if (!std::holds_alternative<calculated_value>(val)) {
				return default_value;
			}

			auto result = std::get_if<typename stored_type<T>::type>(
			    &std::get<calculated_value>(val));
			if (!result) {
				return default_value;
			}

			return *result;
		}

		global_object& operator=(maybe_list<calculated_value>&& data) noexcept {
			value = std::move(data);
			return *this;
		}

		global_object& operator=(contribution const& data) noexcept {
			insert(var::payer, data.payer);
			insert(var::insured, data.insured);
			return *this;
		}

		void debug_print(size_t indent) const {
			if (value) {
				fmt::print(" ");
				std::visit(value_printer<calculated_value>{}, *value);
			} else if (children.empty()) {
				fmt::print(" <null>");
			}
			fmt::print("\n");

			for (auto const& [key, child] : children) {
				fmt::print("{:{}} {}:", "", indent * 2, key);
				child.debug_print(indent + 1);
			}
		}
	};
}  // namespace quick_dra
