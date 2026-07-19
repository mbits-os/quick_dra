// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QLabel>

namespace quick_dra::gui {
	class ToolTipLabel : public QLabel {
		Q_OBJECT

	public:
		ToolTipLabel(const QString& text, const QPoint& pos, QWidget* parent);

		void hideTip();

	protected:
		bool eventFilter(QObject*, QEvent*) override;
		void paintEvent(QPaintEvent* e) override;
		void resizeEvent(QResizeEvent* e) override;
	};
}  // namespace quick_dra::gui
