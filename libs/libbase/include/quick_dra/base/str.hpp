// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace quick_dra {
	using std::literals::operator""s;
	using std::literals::operator""sv;

	template <typename Result, typename InputView>
	inline Result conv(InputView view) {
		using DstChar = typename Result::value_type;
		using SrcChar = typename InputView::value_type;
		static_assert(std::is_trivially_copyable_v<SrcChar>,
		              "conv requires trivially copyable source character type");
		static_assert(
		    std::is_trivially_copyable_v<DstChar>,
		    "conv requires trivially copyable destination character type");
		static_assert(sizeof(SrcChar) == sizeof(DstChar),
		              "conv requires source and destination character types of "
		              "equal size");
		return Result{reinterpret_cast<DstChar const*>(view.data()),
		              view.size()};
	}

	inline auto as_u8v(std::string_view view) {
		return conv<std::u8string_view>(view);
	}
	inline auto as_sv(std::u8string_view view) {
		return conv<std::string_view>(view);
	}
	inline auto as_str(std::u8string_view view) {
		return conv<std::string>(view);
	}
	inline auto as_str(std::string_view view) {
		return conv<std::string>(view);
	}

	struct sep_view_t {
		std::string_view value;
		constexpr explicit sep_view_t(std::string_view value) noexcept
		    : value{value} {}

		constexpr auto empty() const noexcept { return value.empty(); }
		constexpr auto size() const noexcept { return value.size(); }
		void append_to(std::string& result) const { result.append(value); }
	};

	inline consteval sep_view_t operator""_sep(char const* value,
	                                           size_t length) noexcept {
		return sep_view_t{std::string_view{value, length}};
	}

	struct sep_char_t {
		char value;
		constexpr explicit sep_char_t(char value) noexcept : value{value} {}
		constexpr auto empty() const noexcept { return false; }
		constexpr auto size() const noexcept { return 1; }
		void append_to(std::string& result) const { result.push_back(value); }
	};

	inline consteval sep_char_t operator""_sep(char value) noexcept {
		return sep_char_t{value};
	}

	std::vector<std::string_view> split_sv(std::string& data,
	                                       sep_view_t sep,
	                                       size_t max = std::string::npos);
	std::vector<std::string_view> split_sv(std::string_view data,
	                                       sep_view_t sep,
	                                       size_t max = std::string::npos);
	std::vector<std::string> split_s(std::string_view data,
	                                 sep_view_t sep,
	                                 size_t max = std::string::npos);
	std::vector<std::string_view> split_sv(std::string& data,
	                                       sep_char_t sep,
	                                       size_t max = std::string::npos);
	std::vector<std::string_view> split_sv(std::string_view data,
	                                       sep_char_t sep,
	                                       size_t max = std::string::npos);
	std::vector<std::string> split_s(std::string_view data,
	                                 sep_char_t sep,
	                                 size_t max = std::string::npos);

	std::string_view strip_sv(std::string_view);
	std::string_view lstrip_sv(std::string_view);
	std::string_view rstrip_sv(std::string_view);

	std::string strip_s(std::string_view);
	std::string lstrip_s(std::string_view);
	std::string rstrip_s(std::string_view);

	std::string& to_lower_inplace(std::string&) noexcept;
	std::string& to_upper_inplace(std::string&) noexcept;

	std::string to_upper_copy(std::string_view);

	std::string join(std::vector<std::string_view> const& items,
	                 sep_view_t sep);
	std::string join(std::vector<std::string> const& items, sep_view_t sep);
	std::string join(std::vector<std::string_view> const& items,
	                 sep_char_t sep);
	std::string join(std::vector<std::string> const& items, sep_char_t sep);
}  // namespace quick_dra
