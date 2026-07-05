// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <app/controls/PanelButton.hpp>
#include <app/controls/PanelButton_p.hpp>

#include <QPainter>
#include <QPainterPath>

namespace quick_dra::gui {
	namespace {
		void clear(QLayoutItem* item);
		void clear(QLayout* layout) {
			while (layout->count() > 0) {
				auto item = layout->takeAt(layout->count() - 1);
				clear(item);
				delete item;
			}
		}

		void clear(QLayoutItem* item) {
			if (auto widget = item->widget()) {
				widget->deleteLater();
			} else if (auto layout = item->layout()) {
				clear(layout);
				layout->deleteLater();
			}
		}
	}  // namespace

	PanelButtonPrivate::~PanelButtonPrivate() {
		if (has_item_ownership) {
			delete item;
		}
	}

	void PanelButtonPrivate::updateToolTip() {
		toolTip_.clear();
		toolTip_.reserve(buttonTip.size() + keyTip.size() + 1);
		toolTip_.append(buttonTip);
		if (!buttonTip.isEmpty() && !keyTip.isEmpty()) {
			toolTip_.append(u'\t');
		}
		toolTip_.append(keyTip);
	}

	void PanelButtonPrivate::setSequences(QList<QKeySequence> const& sequences) {
		if (sequences.isEmpty()) {
			keyTip.clear();
		} else {
			keyTip = sequences.front().toString(QKeySequence::NativeText);
		}
		updateToolTip();
	}

	void PanelButtonPrivate::setToolTip(QString const& text) {
		buttonTip = text;
		updateToolTip();
	}

	void PanelButtonPrivate::paint(QPainter& painter,
	                               DevicePixelScale const& scale,
	                               Positions pos,
	                               PanelButtonStyle::Palette const& palette) const {
		using namespace PanelButtonStyle;
		auto const radius = scale.toDeviceF(Radius);
		auto const diameter = radius * 2;
		auto const margin = scale.toDeviceF(TrueMargin) + qreal{.5};

		auto const rect = item->geometry().toRectF().marginsAdded({margin, margin, margin, margin});
		QPainterPath path{};
		if (pos & PanePosition::Bottom) {
			path.moveTo(rect.left(), rect.bottom() - radius);
		} else {
			path.moveTo(rect.left(), rect.bottom());
		}

		if (pos & PanePosition::Top) {
			// RIGHT
			path.lineTo(rect.left(), rect.top() + radius);
			// TL corner
			path.arcTo(rect.left(), rect.top(), diameter, diameter, 180, -90);
			// TOP
			path.lineTo(rect.right() - radius, rect.top());
			// TR corner
			path.arcTo(rect.right() - diameter, rect.top(), diameter, diameter, 90, -90);
		} else {
			// RIGHT
			path.lineTo(rect.left(), rect.top());
			// TOP
			path.lineTo(rect.right(), rect.top());
		}

		if (pos & PanePosition::Bottom) {
			path.lineTo(rect.right(), rect.bottom() - radius);
			path.arcTo(rect.right() - diameter, rect.bottom() - diameter, diameter, diameter, 0, -90);
			path.lineTo(rect.left() + radius, rect.bottom());
			path.arcTo(rect.left(), rect.bottom() - diameter, diameter, diameter, 270, -90);
		} else {
			path.lineTo(rect.right(), rect.bottom());
		}

		auto const normal = !isClickable() || (!isActive() && !isHovered());
		auto const pane = !isEnabled() ? palette.disabled
		                  : normal     ? palette.normal
		                  : isActive() ? palette.active
		                               : palette.hover;

		painter.setPen(palette.frame);
		painter.setBrush(pane);
		painter.drawPath(path);
	}

	PanelButton::PanelButton() : d_ptr{new PanelButtonPrivate} {
		Q_D(PanelButton);
		d->q_ptr = this;
	}

	PanelButton::~PanelButton() = default;

	void PanelButton::setSequences(QList<QKeySequence> const& sequences) { d_ptr->setSequences(sequences); }

	QWidget* PanelButton::widget() const { return d_func()->item->widget(); }
	QLayout* PanelButton::layout() const { return d_func()->item->layout(); }

	void PanelButton::clearItem() {
		Q_D(PanelButton);
		clear(d->item);
	}

#define FWD(CLS, NAME)                                                   \
	bool CLS::is##NAME() const noexcept { return d_func()->is##NAME(); } \
	void CLS::set##NAME(bool value) noexcept { d_func()->set##NAME(value); }
	FWD(PanelButton, Clickable)
	FWD(PanelButton, Enabled)
	FWD(PanelButton, Hovered)
	FWD(PanelButton, Active)
#undef GWD

	QString const& PanelButton::toolTip() const noexcept { return d_func()->toolTip(); }
	void PanelButton::setToolTip(QString const& text) { d_func()->setToolTip(text); }
}  // namespace quick_dra::gui
