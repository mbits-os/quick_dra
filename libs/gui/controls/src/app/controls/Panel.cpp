// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QEvent>
#include <QPainter>
#include <QResizeEvent>
#include <algorithm>
#include <app/controls/Panel.hpp>
#include <app/controls/Panel_p.hpp>
#include <app/utils/utils.hpp>

namespace quick_dra::gui {
	bool PanelPrivate::Value::setText(QString const& next, QFont const& font) {
		if (next == text) {
			return false;
		}
		text = next;
		measure(font);
		return true;
	}

	void PanelPrivate::Value::measure(QFont const& font) {
		if (text.isEmpty()) {
			lineHeight.ascent = 0;
			lineHeight.descent = 0;
			lineHeight.leading = 0;
			boundingBox = QRectF{};
			return;
		}

		auto const fm = QFontMetricsF{font};
		lineHeight.measure(fm);
		auto const box = fm.boundingRect(text);
		auto const xMin = std::min(qreal{0}, box.left());
		auto const xMax = std::max(fm.horizontalAdvance(text), box.right());
		boundingBox = QRectF{xMin, box.top(), xMax - xMin, box.height()};
	}

	void PanelPrivate::Value::paint(QPainter* painter) {
		qreal posY = std::max(lineHeight.ascent, -boundingBox.top());
		qreal posX = -boundingBox.left();

		painter->drawText(QPointF{posX + offset.x(), posY + offset.y()}, text);
	}

	class PanelPrivate::HeightForWidthHandler {
	public:
		void moveValue(auto&&...) const {}
		auto labelHeight(qreal, qreal, qreal width, MarkdownLabel& label) const {
			return static_cast<qreal>(label.heightForWidth(static_cast<int>(width)));
		}
		void adjustHeightChange(auto&&...) const {}
	};

	void PanelPrivate::setTitle(QString const& text) {
		ui.title.setText(text);
		if (ui.title.needsLayout()) {
			requestOptimalSizes();
			requestLayout();
		}
	}

	void PanelPrivate::setDetails(QString const& text) {
		ui.details.setText(text);
		if (ui.details.needsLayout()) {
			requestOptimalSizes();
			requestLayout();
		}
	}

	void PanelPrivate::setValue(QString const& text) {
		if (ui.value.setText(text, q_ptr->font())) {
			requestOptimalSizes();
			requestLayout();
		}
	}

	void PanelPrivate::setIcon(QIcon const& icon) {
		ui.icon = icon;
		requestLayout();
	}

	void PanelPrivate::updateFont() {
		auto const& font = q_ptr->font();
		ui.value.measure(font);
		ui.title.setFont(font);
		ui.details.setFont(font);
		requestOptimalSizes();
		requestLayout();
	}

	void PanelPrivate::setWidth(int width) {
		if (currentWidth_ == width) {
			return;
		}
		currentWidth_ = width;
		requestLayout();
	}

	int PanelPrivate::heightForWidth(int width) { return qRound(layoutImpl<HeightForWidthHandler>(width)); }

	QSize const& PanelPrivate::sizeHint() {
		layout();
		return hintedSize_;
	}

	QSize const& PanelPrivate::minimumSizeHint() {
		layout();
		return minimalSize_;
	}

	void PanelPrivate::requestOptimalSizes() { optimalSizesInvalid_ = true; }

	void PanelPrivate::requestLayout() {
		if (needsLayout_) return;
		needsLayout_ = true;
		q_ptr->updateGeometry();
	}

	void PanelPrivate::layout() {
		if (!needsLayout_) return;
		needsLayout_ = false;
		doLayout();
	}

	class PanelPrivate::LayoutHandler {
	public:
		void moveValue(qreal x, qreal y, Value& value) const { value.offset = {x, y}; }
		auto labelHeight(qreal x, qreal y, qreal width, MarkdownLabel& label) const {
			label.setWidth(static_cast<int>(width));
			label.moveTo({x, y});
			return label.calcSize().height();
		}
		void adjustHeightChange(qreal adjustment, MarkdownLabel& label) const {
			label.moveTo(label.topLeft() + QPointF{0, adjustment});
		}
		void adjustHeightChange(qreal adjustment, Value& value) const {
			value.offset.setY(value.offset.y() + adjustment);
		}
	};

	class PanelPrivate::HintSizer {
	public:
		auto sizeFrom(MarkdownLabel& label) const { return label.calcSingleLineSize(); }
	};

	class PanelPrivate::MinimalSizer {
	public:
		auto sizeFrom(MarkdownLabel& label) const { return label.calcMinimalSize(); }
	};

	void PanelPrivate::doLayout() {
		if (optimalSizesInvalid_) {
			optimalSizesInvalid_ = false;
			hintedSize_ = calcOptimalSize<HintSizer>().toSize();
			minimalSize_ = calcOptimalSize<MinimalSizer>().toSize();
			q_ptr->setMinimumWidth(minimalSize_.width());
		}
		hintedSize_.setHeight(qRound(layoutImpl<LayoutHandler>(currentWidth_)));
	}

	void PanelPrivate::paint(QPainter* painter) {
		// GCOV_EXCL_START
		if (scale.updateScale(q_ptr->logicalDpiX())) {
			[[unlikely]];
			requestLayout();
		}
		// GCOV_EXCL_STOP

		auto const size = q_ptr->rect().size();
		painter->save();
		painter->setClipRect(q_ptr->rect(), Qt::IntersectClip);
		paintIcon(painter, size.width(), size.height());

		auto const hasValue = !ui.value.text.isEmpty();
		auto const hasTitle = !ui.title.cleanText().isEmpty();
		auto const hasDetails = !ui.details.cleanText().isEmpty();

		if (hasValue) ui.value.paint(painter);
		if (hasTitle) ui.title.paint(painter);
		if (hasDetails) ui.details.paint(painter);
		painter->restore();
	}

	void PanelPrivate::paintIcon(QPainter* painter, int width, int height) {
		auto const SIZE = scale.toDevice(ICON_SIZE_PIXELS);
		if (ui.icon.isNull()) return;
		ui.icon.paint(painter, QRect{width - SIZE, 0, SIZE, height}, Qt::AlignCenter,
		              q_ptr->isEnabled() ? QIcon::Active : QIcon::Disabled);
	}

	Panel::Panel(QWidget* parent) : QWidget{parent}, d_ptr{std::make_unique<PanelPrivate>(this)} {
		Q_D(Panel);
		d->updateFont();
		setSizePolicy(TakeWidth / HeightForWidth);
	}

	Panel::~Panel() = default;

	// GCOV_EXCL_START -- this function has been deprecated and will be removed in v1.5.0
	void Panel::setInfo(QString const& label, QString const& details, QString const& value, QIcon const& rightIcon) {
		setInfo({.label = label, .details = details, .value = value, .rightIcon = rightIcon});
	}
	// GCOV_EXCL_STOP

	void Panel::setInfo(PanelInfo const& info) {
		setTitle(info.label);
		setValue(info.value);
		setDetails(info.details);
		setIcon(info.rightIcon);
	}

#define SET(TYPE, NAME)                            \
	void Panel::NAME(TYPE const& value) noexcept { \
		Q_D(Panel);                                \
		d->NAME(value);                            \
	}

	SET(QString, setTitle);
	SET(QString, setValue);
	SET(QString, setDetails);
	SET(QIcon, setIcon);

	bool Panel::hasHeightForWidth() const { return true; }

	int Panel::heightForWidth(int width) const { return d_ptr->heightForWidth(width); }

	QSize Panel::sizeHint() const { return d_ptr->sizeHint(); }

	QSize Panel::minimumSizeHint() const { return d_ptr->minimumSizeHint(); }

	bool Panel::event(QEvent* event) {
		if (event->type() == QEvent::FontChange) {
			d_ptr->updateFont();
		}
		return QWidget::event(event);
	}

	void Panel::resizeEvent(QResizeEvent* event) {
		Q_D(Panel);
		d->setWidth(event->size().width());
	}

	void Panel::paintEvent(QPaintEvent*) {
		QPainter painter{this};
		d_ptr->paint(&painter);
	}
}  // namespace quick_dra::gui
