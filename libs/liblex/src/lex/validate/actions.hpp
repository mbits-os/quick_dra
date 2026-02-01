// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <array>
#include <cctype>
#include <list>
#include <utility>
#include "fixed_string.hpp"

namespace quick_dra::checker {
	template <typename... Action>
	struct list {
		static constexpr auto size_ = sizeof...(Action);
		using weights_t = std::array<int, size_>;

		constexpr auto size() const noexcept { return size_; }

		static bool check(std::string_view str) noexcept {
			return list::check_each(
			    str, std::make_index_sequence<sizeof...(Action)>{});
		}

		static int weighted_sum(std::string_view str,
		                        weights_t const& weights) noexcept {
			return list::sum_values(
			    str, weights, std::make_index_sequence<sizeof...(Action)>{});
		}

	private:
		template <size_t... I>
		static bool check_each(std::string_view str,
		                       std::index_sequence<I...>) noexcept {
			if (str.size() != sizeof...(I)) return false;
			return (Action{}.check(static_cast<unsigned char>(str[I])) && ...);
		}

		template <size_t... I>
		static int sum_values(std::string_view str,
		                      weights_t const& weights,
		                      std::index_sequence<I...>) noexcept {
			if (str.size() != sizeof...(I)) return false;

			return ((Action{}.value(str[I]) * weights[I]) + ... + 0);
		}
	};

	template <std::array Input, typename List>
	struct weights_wrapper {
		using list = List;
		static constexpr auto weights = Input;

		bool check(std::string_view str) const noexcept {
			return list::check(str);
		}

		int weighted_sum(std::string_view str) const noexcept {
			return list::weighted_sum(str, weights);
		}
	};

	template <std::array Input, typename List, typename Lambda>
	struct full_mask_type : Lambda, weights_wrapper<Input, List> {
		unsigned short checksum(std::string_view id) const noexcept {
			if (!this->check(id)) {
				return kInvalidChecksum;
			}

			auto const sum = this->weighted_sum(id);
			return static_cast<unsigned short>(
			    static_cast<Lambda const&>(*this)(sum));
		}
	};

	template <std::array Input, typename List>
	struct mask_type : weights_wrapper<Input, List> {
		template <typename Lambda>
		constexpr full_mask_type<Input, List, Lambda> postproc(
		    Lambda&& cb) const noexcept {
			return {std::forward<Lambda>(cb)};
		}
	};

	template <typename List>
	struct partial_mask_type {
		using list = List;
		static constexpr auto size_ = list::size_;
		using weights_t = typename list::weights_t;

		template <int... Index>
		    requires(sizeof...(Index) == size_)
		constexpr mask_type<std::array{Index...}, list> with() const noexcept {
			return {};
		}
	};

	template <char ch>
	struct action_from_t {};

	template <char ch>
	using action_from = typename action_from_t<ch>::type;

	template <>
	struct action_from_t<'A'> {
		struct upper_case {
			bool check(char ch) const noexcept {
				return std::isalpha(ch) && std::toupper(ch) == ch;
			}

			unsigned value(char ch) const noexcept { return ch - 'A' + 10; }
		};

		using type = upper_case;
	};

	template <>
	struct action_from_t<'0'> {
		struct digit {
			bool check(char ch) const noexcept { return std::isdigit(ch); }

			unsigned value(char ch) const noexcept { return ch - '0'; }
		};

		using type = digit;
	};

	template <>
	struct action_from_t<'F'> {
		struct hex_digit {
			bool check(char ch) const noexcept { return std::isxdigit(ch); }

			unsigned value(char ch) const noexcept {
				switch (ch) {
					case 'A':
					case 'B':
					case 'C':
					case 'D':
					case 'E':
					case 'F':
						return ch - 'A' + 10;
					case 'a':
					case 'b':
					case 'c':
					case 'd':
					case 'e':
					case 'f':
						return ch - 'a' + 10;
				}
				return ch - '0';
			}
		};

		using type = hex_digit;
	};

	template <>
	struct action_from_t<'?'> {
		struct ignore {
			bool check(char) const noexcept { return true; }

			unsigned value(char) const noexcept { return 0; }
		};

		using type = ignore;
	};

	template <fixed_string Input>
	struct mask_builder {
		static constexpr auto input = Input;
		template <size_t... I>
		static auto create(std::index_sequence<I...>) {
			return partial_mask_type<list<action_from<input.get<I>()>...>>{};
		}

		using type = decltype(create(std::make_index_sequence<input.size()>{}));
	};

	template <fixed_string Input>
	constexpr auto mask = typename mask_builder<Input>::type{};
}  // namespace quick_dra::checker
