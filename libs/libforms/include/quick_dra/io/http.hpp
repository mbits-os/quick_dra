// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace quick_dra {
	struct http_response {
		unsigned status{};
		struct {
			std::string type;
			std::string subtype;
		} content_type{};
		std::string charset{};
		std::vector<std::byte> content{};

		// GCOV_EXCL_START
		// Function used exclusively by cUrl exceptional code, itself excluded
		http_response& cleaned() noexcept {
			status = 0;
			content_type.type.clear();
			content_type.subtype.clear();
			charset.clear();
			content.clear();
			return *this;
		}
		// GCOV_EXCL_STOP

		std::string_view text() const noexcept {
			return {reinterpret_cast<char const*>(content.data()),
			        content.size()};
		}

		explicit operator bool() const noexcept { return (status / 100) == 2; }
	};

	http_response http_get(std::string const& url);
}  // namespace quick_dra
