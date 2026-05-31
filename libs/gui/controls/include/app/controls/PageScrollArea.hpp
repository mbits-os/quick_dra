// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QScrollArea>
#include <QWidget>
#include <utility>

namespace quick_dra::gui {
	class PageScrollArea : public QScrollArea {
		Q_OBJECT;

	public:
		explicit PageScrollArea(QWidget* parent = nullptr);

		static std::pair<PageScrollArea*, QWidget*> setupPage(QWidget* page);

		void resizeEvent(QResizeEvent* event) override;
	};
}  // namespace quick_dra::gui
