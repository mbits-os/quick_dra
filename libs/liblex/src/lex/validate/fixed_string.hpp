// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <array>

namespace quick_dra::checker {
	// from CTRE:
	struct construct_from_pointer_t {};
	static constexpr auto construct_from_pointer = construct_from_pointer_t{};

	template <size_t N>
	struct fixed_string {
		char content[N] = {};

		constexpr fixed_string(construct_from_pointer_t,
		                       char const* input) noexcept {
			for (size_t i{0}; i < N; ++i) {
				content[i] = static_cast<uint8_t>(input[i]);
			}
		}

		constexpr fixed_string(std::array<char, N> const& in) noexcept
		    : fixed_string{construct_from_pointer, in.data()} {}
		constexpr fixed_string(char const (&input)[N + 1]) noexcept
		    : fixed_string{construct_from_pointer, input} {}

		constexpr fixed_string(const fixed_string& other) noexcept {
			for (size_t i{0}; i < N; ++i) {
				content[i] = other.content[i];
			}
		}
		constexpr size_t size() const noexcept { return N; }
		constexpr const char* begin() const noexcept { return content; }
		constexpr const char* end() const noexcept { return content + size(); }
		template <size_t I>
		constexpr char get() const noexcept {
			if constexpr (I < N) {
				return content[I];
			}
		}
		constexpr char operator[](size_t i) const noexcept {
			return content[i];
		}
		constexpr operator std::basic_string_view<char>() const noexcept {
			return std::basic_string_view<char>{content, size()};
		}
	};

	template <>
	class fixed_string<0> {
		static constexpr char empty[1] = {0};

	public:
		constexpr fixed_string(char const*) noexcept {}
		constexpr fixed_string(std::initializer_list<char>) noexcept {}
		constexpr fixed_string(const fixed_string&) noexcept {}
		constexpr bool correct() const noexcept { return true; }
		constexpr size_t size() const noexcept { return 0; }
		constexpr char const* begin() const noexcept { return empty; }
		constexpr char const* end() const noexcept { return empty + size(); }
		constexpr char operator[](size_t) const noexcept { return 0; }
		constexpr operator std::basic_string_view<char>() const noexcept {
			return std::basic_string_view<char>{empty, 0};
		}
	};

	template <size_t N>
	fixed_string(char const (&)[N]) -> fixed_string<N - 1>;
	template <size_t N>
	fixed_string(const std::array<char, N>&) -> fixed_string<N>;

	template <size_t N>
	fixed_string(fixed_string<N>) -> fixed_string<N>;
}  // namespace quick_dra::checker
