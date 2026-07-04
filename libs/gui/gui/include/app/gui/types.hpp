// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <app/utils/DevicePixelScale.hpp>
#include <chrono>

using namespace std::literals;

namespace quick_dra::gui {
	enum class PageChangeDirection {
		Push,
		Pop,
	};

	namespace PageChangeAnimation {
		static constexpr auto Duration = 150ms;
		static constexpr auto ProgressFrom = qreal{};
		static constexpr auto ProgressTo = qreal{1.0};
		static constexpr auto SlideLength = 12_px;
	}  // namespace PageChangeAnimation
}  // namespace quick_dra::gui
