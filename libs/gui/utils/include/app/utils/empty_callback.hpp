// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

namespace quick_dra::gui {
	template <typename T>
	struct EmptyCallback {
		void operator()(T const&) {}
	};
}  // namespace quick_dra::gui
