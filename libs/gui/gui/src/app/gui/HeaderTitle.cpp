// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QFontMetrics>
#include <QPainter>
#include <app/gui/HeaderTitle.hpp>
#include <app/gui/HeaderTitle_p.hpp>

namespace quick_dra::gui {

	void HeaderTitlePrivate::setText(const QString& value) {
		if (text_ == value) return;
		text_ = value;
		auto const fm = QFontMetrics{q_ptr->font()};
		hint_ = fm.boundingRect(text_).size();
		q_ptr->update();
		q_ptr->updateGeometry();
	}

	void HeaderTitlePrivate::setAlignment(Qt::Alignment value) {
		if (alignment_ == value) return;
		alignment_ = value;
		q_ptr->update();
	}

	void HeaderTitlePrivate::setAnimationProgress(qreal value) {
		if (animation.progress == value) return;
		animation.progress = value;
		if (animation.progress == PageChangeAnimation::ProgressTo) {
			animation.old.clear();
			animation.setDirection(0);
		}
		q_ptr->update();
	}

	void HeaderTitlePrivate::paint(QPainter* painter) {
		auto const pos = q_ptr->rect();
		if (!animation.isActive()) {
			painter->drawText(pos, text_, alignment_);
			return;
		}

		auto textColor = q_ptr->palette().color(QPalette::WindowText);
		animation.drawOld(painter, pos, alignment_, textColor);
		animation.drawCurrent(painter, pos, alignment_, textColor, text_);
	}

	void HeaderTitlePrivate::AnimationInfo::before(PageChangeDirection dir, QString const& text) {
		old = text;
		progress = PageChangeAnimation::ProgressFrom;
		setDirection(dir == PageChangeDirection::Push ? 1 : -1);
	}

	bool HeaderTitlePrivate::AnimationInfo::isActive() const { return !!direction; }

	void HeaderTitlePrivate::AnimationInfo::setDirection(int value) {
		if (value == direction) return;
		direction = value;
		parent->q_ptr->animationDirectionChange(direction);
	}

	qreal HeaderTitlePrivate::AnimationInfo::slideToDeviceF(QPainter* painter) const {
		return DevicePixelScale{painter->device()->logicalDpiX()}.toDeviceF(PageChangeAnimation::SlideLength);
	}

	void HeaderTitlePrivate::AnimationInfo::drawCurrent(QPainter* painter,
	                                                    QRect const& pos,
	                                                    Qt::Alignment alignment,
	                                                    QColor const& textColor,
	                                                    QString const& text) {
		auto const maxSlide = slideToDeviceF(painter);
		auto const slide = maxSlide * progress;
		drawText(painter, pos, alignment, textColor, text, qRound(maxSlide - slide) * direction,
		         static_cast<float>(progress));
	}

	void HeaderTitlePrivate::AnimationInfo::drawOld(QPainter* painter,
	                                                QRect const& pos,
	                                                Qt::Alignment alignment,
	                                                QColor const& textColor) {
		auto const maxSlide = slideToDeviceF(painter);
		auto const slide = maxSlide * progress;
		drawText(painter, pos, alignment, textColor, old, -qRound(slide) * direction,
		         static_cast<float>(PageChangeAnimation::ProgressTo - progress));
	}

	void HeaderTitlePrivate::AnimationInfo::drawText(QPainter* painter,
	                                                 QRect const& pos,
	                                                 Qt::Alignment alignment,
	                                                 QColor textColor,
	                                                 QString const& text,
	                                                 int dx,
	                                                 float alphaChange) {
		textColor.setAlphaF(textColor.alphaF() * alphaChange);
		painter->setPen(textColor);
		painter->drawText(pos.translated(dx, 0), text, alignment);
	}

	HeaderTitle::HeaderTitle(QWidget* parent) : QWidget{parent}, q_ptr{std::make_unique<HeaderTitlePrivate>(this)} {}
	HeaderTitle::~HeaderTitle() = default;

	void HeaderTitle::setText(const QString& value) { q_ptr->setText(value); }
	void HeaderTitle::setAlignment(Qt::Alignment value) { q_ptr->setAlignment(value); }
	void HeaderTitle::setAnimationProgress(qreal value) { q_ptr->setAnimationProgress(value); }
	qreal HeaderTitle::animationProgress() const { return q_ptr->animationProgress(); }

	QSize HeaderTitle::sizeHint() const { return q_ptr->sizeHint(); }
	QSize HeaderTitle::minimumSizeHint() const { return q_ptr->minimumSizeHint(); }

	void HeaderTitle::paintEvent(QPaintEvent*) {
		QPainter painter{this};
		painter.setClipRect(rect());
		q_ptr->paint(&painter);
	}

	void HeaderTitle::beforeAnimation(PageChangeDirection dir) { q_ptr->beforeAnimation(dir); }
}  // namespace quick_dra::gui
