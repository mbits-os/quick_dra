// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <webui.h>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace quick_dra::gui::webui {
	struct free {
		void operator()(void* ptr) { ::webui_free(ptr); }
	};

	template <typename T>
	using ptr = std::unique_ptr<T, free>;

	template <typename T>
	inline ptr<T> make_ptr(size_t size) {
		return ptr<T>{static_cast<T*>(::webui_malloc(size * sizeof(T)))};
	}
}  // namespace quick_dra::gui::webui
