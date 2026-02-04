// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#define WINDOW
#include <Windows.h>
#include <quick_dra/base/str.hpp>
#include <span>

namespace quick_dra {
	namespace {
		auto conv_one_way(std::span<char const> const& src,
		                  wchar_t* dst,
		                  DWORD size) {
			return MultiByteToWideChar(CP_UTF8, 0, src.data(),
			                           static_cast<DWORD>(src.size()), dst,
			                           size);
		}
		auto conv_one_way(std::span<wchar_t const> const& src,
		                  char* dst,
		                  DWORD size) {
			return WideCharToMultiByte(CP_UTF8, 0, src.data(),
			                           static_cast<DWORD>(src.size()), dst,
			                           size, nullptr, nullptr);
		}
		auto lc_map(std::span<wchar_t const> const& src,
		            wchar_t* dst,
		            DWORD size) {
			return LCMapStringW(LOCALE_USER_DEFAULT, LCMAP_UPPERCASE,
			                    src.data(), static_cast<DWORD>(src.size()), dst,
			                    size);
		}

		template <typename StringLike>
		struct opposite {};
		template <>
		struct opposite<std::string> {
			using type = std::wstring;
		};
		template <>
		struct opposite<std::string_view> : opposite<std::string> {
			using type = std::wstring;
		};
		template <>
		struct opposite<std::wstring> {
			using type = std::string;
		};

		template <typename StringLike>
		using opposite_t = typename opposite<StringLike>::type;

		template <typename StringLike>
		opposite_t<StringLike> conv(StringLike const& view) {
			auto size = conv_one_way(view, nullptr, 0);
			opposite_t<StringLike> result(static_cast<size_t>(size), '\0');
			conv_one_way(view, result.data(), size);
			return result;
		}
	}  // namespace
	std::string to_upper(std::string_view input) {
		if (input.empty()) return {};

		auto wide = conv(input);
		auto const size = lc_map(wide, nullptr, 0);
		std::wstring upper(size, L'\0');
		lc_map(wide, upper.data(), size);
		return conv(upper);
	}
}  // namespace quick_dra
