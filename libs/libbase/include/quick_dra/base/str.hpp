// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <string>
#include <string_view>
#include <vector>

using namespace std::literals;

namespace quick_dra {
	template <typename Result, typename InputView>
	inline Result conv(InputView view) {
		return Result{
		    reinterpret_cast<typename Result::value_type const*>(view.data()),
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

	std::vector<std::string_view> split_sv(std::string_view sep,
	                                       std::string& data,
	                                       size_t max = std::string::npos);
	std::vector<std::string_view> split_sv(std::string_view sep,
	                                       std::string_view data,
	                                       size_t max = std::string::npos);
	std::vector<std::string> split_s(std::string_view sep,
	                                 std::string_view data,
	                                 size_t max = std::string::npos);
	std::vector<std::string_view> split_sv(char sep,
	                                       std::string& data,
	                                       size_t max = std::string::npos);
	std::vector<std::string_view> split_sv(char sep,
	                                       std::string_view data,
	                                       size_t max = std::string::npos);
	std::vector<std::string> split_s(char sep,
	                                 std::string_view data,
	                                 size_t max = std::string::npos);

	std::string_view strip_sv(std::string_view);
	std::string_view lstrip_sv(std::string_view);
	std::string_view rstrip_sv(std::string_view);

	std::string strip_s(std::string_view);
	std::string lstrip_s(std::string_view);
	std::string rstrip_s(std::string_view);

	std::string& tolower_inplace(std::string&) noexcept;
	std::string& toupper_inplace(std::string&) noexcept;

	std::string toupper_flt(std::string_view);

	std::string join(std::string_view sep,
	                 std::vector<std::string_view> const& items);
	std::string join(std::string_view sep,
	                 std::vector<std::string> const& items);
}  // namespace quick_dra
