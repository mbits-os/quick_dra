// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QSize>
#include <QToolButton>
#include <app/utils/utils.hpp>

namespace quick_dra::gui {
	void restrictToolButton(QWidget* button, int size) {
		button->setBaseSize(QSize(size, size));
		button->setMinimumSize(QSize(size, size));
		button->setMaximumSize(QSize(size, size));
	}
}  // namespace quick_dra::gui
