// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

// #define U_DISABLE_RENAMING 1
#include <unicode/unistr.h>

#include <quick_dra/base/str.hpp>
#include <string>

namespace quick_dra {
	std::string to_upper(std::string_view input) {
		std::string result{};
		icu::UnicodeString::fromUTF8(input).toUpper().toUTF8String(result);
		return result;
	}  // GCOV_EXCL_LINE
}  // namespace quick_dra
