// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QIcon>
#include <QWidget>

namespace quick_dra::gui {
	class Glyph : public QWidget {
		Q_OBJECT

	public:
		explicit Glyph(QWidget* parent = nullptr);

		void paintEvent(QPaintEvent*) override;
		QSize sizeHint() const override;
		QSize minimumSizeHint() const override { return sizeHint(); }

		QIcon const& icon() const noexcept { return icon_; }
		Qt::Alignment alignment() const noexcept { return alignment_; }

	public slots:
		void setIcon(QIcon const&);
		void setSize(int, int);
		void setAlignment(Qt::Alignment);

	private:
		QIcon icon_{};
		QSize intrinsicSize_{0, 0};
		QSize size_{-1, -1};
		Qt::Alignment alignment_{Qt::AlignCenter};
	};
}  // namespace quick_dra::gui
