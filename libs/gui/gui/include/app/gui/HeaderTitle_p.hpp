// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QWidget>
#include <app/gui/HeaderTitle.hpp>
#include <app/gui/types.hpp>
#include <app/utils/DevicePixelScale.hpp>

namespace quick_dra::gui {
	class HeaderTitlePrivate {
	public:
		explicit HeaderTitlePrivate(HeaderTitle* q_ptr) : q_ptr{q_ptr} {}

		void setText(const QString&);
		void setAlignment(Qt::Alignment value);
		void setAnimationProgress(qreal value);
		qreal animationProgress() const { return animation.progress; }

		QSize sizeHint() { return hint_; }
		QSize minimumSizeHint() { return sizeHint(); }
		void paint(QPainter* painter);

		void beforeAnimation(PageChangeDirection dir) { animation.before(dir, text_); }

	private:
		HeaderTitle* q_ptr;

		struct AnimationInfo {
			HeaderTitlePrivate* parent{};
			int direction{};
			qreal progress{PageChangeAnimation::ProgressTo};
			QString old{};

			void before(PageChangeDirection, QString const&);
			bool isActive() const;
			void setDirection(int direction);
			qreal slideToDeviceF(QPainter*) const;
			void drawCurrent(QPainter*, QRect const&, Qt::Alignment, QColor const&, QString const&);
			void drawOld(QPainter*, QRect const&, Qt::Alignment, QColor const&);
			void drawText(QPainter*, QRect const&, Qt::Alignment, QColor, QString const&, int dx, float alphaChange);
		};

		QSize hint_{};
		QString text_{};
		Qt::Alignment alignment_{};
		AnimationInfo animation{.parent = this};
	};
}  // namespace quick_dra::gui
