// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <qtypes.h>
#include <QPainter>
#include <QVBoxLayout>
#include <app/utils/DevicePixelScale.hpp>
#include <vector>
#include "PanelButtonGroup.hpp"

namespace quick_dra::gui::PanelButtonStyle {
	static constexpr auto TrueMargin = 15_px;
	static constexpr auto Margin = TrueMargin + 2_px;
	static constexpr auto Radius = 6_px;

	struct Palette {
		QColor frame{};
		QColor normal{};
		QColor hover{};
		QColor active{};
		QColor disabled{};
	};
}  // namespace quick_dra::gui::PanelButtonStyle
