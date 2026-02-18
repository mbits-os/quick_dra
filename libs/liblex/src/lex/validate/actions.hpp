// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <array>
#include <cctype>
#include <list>
#include <utility>
#include "check_compiler.hpp"
#include "fixed_string.hpp"

#if CNTTP_COMPILER_CHECK
#define CHECKER_INPUT_TYPE fixed_string
#define CHECKER_TEMPLATE_COPY_TYPE auto
#else
#define CHECKER_INPUT_TYPE const auto&
#define CHECKER_TEMPLATE_COPY_TYPE const auto&
#endif

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

			return ((Action{}.value(static_cast<unsigned char>(str[I])) *
			         weights[I]) +
			        ... + 0);
		}
	};

	template <CHECKER_TEMPLATE_COPY_TYPE Input, typename List>
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

	template <CHECKER_TEMPLATE_COPY_TYPE Input,
	          typename List,
	          typename PostprocLambda>
	struct full_mask_type : PostprocLambda, weights_wrapper<Input, List> {
		unsigned short checksum(std::string_view id) const noexcept {
			if (!this->check(id)) {
				return kInvalidChecksum;
			}

			auto const sum = this->weighted_sum(id);
			return static_cast<unsigned short>(
			    static_cast<PostprocLambda const&>(*this)(sum));
		}
	};

	template <CHECKER_TEMPLATE_COPY_TYPE Input, typename List>
	struct mask_type : weights_wrapper<Input, List> {
		template <typename PostprocLambda>
		// GCOV_EXCL_START[CLANG]
		// This function is used, including tests, and still marked by llvm as
		// not visited
		constexpr full_mask_type<Input, List, PostprocLambda> postproc(
		    PostprocLambda&& cb) const noexcept {
			return {std::forward<PostprocLambda>(cb),
			        weights_wrapper<Input, List>{}};
		}
		// GCOV_EXCL_STOP
	};

	template <typename List>
	struct partial_mask_type {
		using list = List;
		static constexpr auto size_ = list::size_;
		using weights_t = typename list::weights_t;

		template <int... Index>
		    requires(sizeof...(Index) == size_)
		// GCOV_EXCL_START[CLANG]
		// This function is used, including tests, and still marked by llvm as
		// not visited
		constexpr mask_type<std::array{Index...}, list> with() const noexcept {
			return {};
		}
		// GCOV_EXCL_STOP
	};

	template <char ch>
	struct action_from_t {};

	template <CHECKER_TEMPLATE_COPY_TYPE Input, size_t Index>
	using action_from =
	    typename action_from_t<Input.template get<Index>()>::type;

	template <>
	struct action_from_t<'A'> {
		struct upper_case {
			bool check(unsigned char ch) const noexcept {
				return std::isalpha(ch) && std::toupper(ch) == ch;
			}

			int value(unsigned char ch) const noexcept { return ch - 'A' + 10; }
		};

		using type = upper_case;
	};

	template <>
	struct action_from_t<'0'> {
		struct digit {
			bool check(unsigned char ch) const noexcept {
				return std::isdigit(ch);
			}

			int value(unsigned char ch) const noexcept { return ch - '0'; }
		};

		using type = digit;
	};

	template <>
	struct action_from_t<'?'> {
		struct ignore {
			bool check(unsigned char) const noexcept { return true; }

			int value(unsigned char) const noexcept { return 0; }
		};

		using type = ignore;
	};

	template <CHECKER_TEMPLATE_COPY_TYPE Input>
	struct mask_builder {
		static constexpr auto input = fixed_string{Input};
		// GCOV_EXCL_START[CLANG]
		// This function is designed to only be part of decltype, never called
		template <size_t... I>
		static auto create(std::index_sequence<I...>) {
			return partial_mask_type<list<action_from<input, I>...>>{};
		}
		// GCOV_EXCL_STOP

		using type = decltype(create(std::make_index_sequence<input.size()>{}));
	};

	template <CHECKER_INPUT_TYPE Input>
	constexpr auto mask = typename mask_builder<Input>::type{};
}  // namespace quick_dra::checker
