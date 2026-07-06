// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <app/controls/PanelButton.hpp>
#include <app/controls/PanelButton_p.hpp>

#include <QPainter>
#include <QPainterPath>
#include <QScrollArea>

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

		QScrollArea* scrollAreaParent(QWidget* ptr) {
			while (ptr) {
				auto result = qobject_cast<QScrollArea*>(ptr);
				if (result) return result;
				ptr = qobject_cast<QWidget*>(ptr->parent());
			}

			return nullptr;
		}
	}  // namespace

	Shortcuts::Shortcuts(PanelButtonPrivate* parent) : QObject{parent} {}

	void Shortcuts::setEnabled(bool value) {
		if (value == enabled) return;
		auto const prevEnabledAndFocused = enabled && focused;
		enabled = value;
		auto const enabledAndFocused = enabled && focused;
		if (enabledAndFocused == prevEnabledAndFocused) return;
		enableShortcuts(enabledAndFocused);
	}

	void Shortcuts::setFocused(bool value) {
		if (value == focused) return;
		auto const prevEnabledAndFocused = enabled && focused;
		focused = value;
		auto const enabledAndFocused = enabled && focused;
		if (enabledAndFocused == prevEnabledAndFocused) return;
		enableShortcuts(enabledAndFocused);
	}

	void Shortcuts::enableShortcuts(bool enabledAndFocused) {
		for (auto shortcut : shortcuts) {
			shortcut->setEnabled(enabledAndFocused);
		}
	}

	void Shortcuts::setSequences(QList<QKeySequence> const& keys) {
		for (auto const& shortcut : shortcuts) {
			shortcut->setEnabled(false);
			shortcut->setParent(nullptr);
			shortcut->deleteLater();
		}

		shortcuts.clear();
		shortcuts.reserve(static_cast<size_t>(keys.size()));
		for (auto const& key : keys) {
			shortcuts.push_back(new QShortcut{key, parent(), this, &Shortcuts::activated,
			                                  &Shortcuts::activatedAmbiguously, Qt::ApplicationShortcut});
		}

		enableShortcuts(enabled && focused);

		sequences = keys;
	}

	void Shortcuts::activated() {
		auto const d = static_cast<PanelButtonPrivate*>(parent());
		d->q_func()->clicked();
	}

	void Shortcuts::activatedAmbiguously() {
		auto const d = static_cast<PanelButtonPrivate*>(parent());
		qWarning() << "Ambiguous call to" << d->toolTip();
		d->q_func()->clicked();
	}

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
		shortcuts->setSequences(sequences);
	}

	void PanelButtonPrivate::setToolTip(QString const& text) {
		buttonTip = text;
		updateToolTip();
	}

	QRect PanelButtonPrivate::geometry() const {
		auto const margin = q_parent->scale().toDevice(PanelButtonStyle::TrueMargin);
		return item->geometry().marginsAdded({margin, margin, margin, margin});
	}

	void PanelButtonPrivate::ensureVisible() const {
		auto scrollArea = scrollAreaParent(q_parent);
		if (!scrollArea) {  // GCOV_EXCL_LINE
			[[unlikely]];   // GCOV_EXCL_LINE
			return;         // GCOV_EXCL_LINE
		}

		auto const scrolledWidget = scrollArea->widget();
		auto const geo = geometry();
		auto const focus = QRect{q_parent->mapTo(scrolledWidget, geo.topLeft()), geo.size()};
		scrollArea->ensureVisible(focus.left(), (focus.top() + focus.bottom()) / 2, 0, focus.height() * 3 / 2);
	}

	QPainterPath plotPath(QRectF const& rect, qreal radius, Positions pos, bool close) {
		auto const diameter = radius * 2;

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

			if (close) path.lineTo(rect.left(), rect.bottom());
		}

		return path;
	}  // GCOV_EXCL_LINE

	void PanelButtonPrivate::paint(QPainter& painter,
	                               DevicePixelScale const& scale,
	                               Positions pos,
	                               PanelButtonStyle::Palette const& palette) const {
		using namespace PanelButtonStyle;
		auto const radius = scale.toDeviceF(Radius);
		auto const margin = scale.toDeviceF(TrueMargin) + qreal{.5};

		auto const rect = item->geometry().toRectF().marginsAdded({margin, margin, margin, margin});

		auto const normal = !isClickable() || (!isActive() && !isHovered());
		auto const pane = !isEnabled() ? palette.disabled
		                  : normal     ? palette.normal
		                  : isActive() ? palette.active
		                               : palette.hover;

		painter.setPen(palette.frame);
		painter.setBrush(pane);
		painter.drawPath(plotPath(rect, radius, pos, false));
	}

	void PanelButtonPrivate::paintFocusRect(QPainter& painter, DevicePixelScale const& scale, Positions pos) const {
		using namespace PanelButtonStyle;
		auto const trueMargin = scale.toDeviceF(TrueMargin) + qreal{.5};
		auto const fullMargin = scale.toDeviceF(Margin) + qreal{.5};
		auto const margin = (trueMargin + fullMargin) / 2;
		auto const radius = scale.toDeviceF(Radius) + (margin - trueMargin);
		auto const dist = scale.toDeviceF(FocusMarker) + qreal{.5};

		auto const rect =
		    item->geometry().toRectF().marginsAdded({margin, margin, margin, margin - scale.toDeviceF(1_px) - 1});
		auto const markerLeft = rect.left() + dist + qreal{.5};
		auto path = plotPath(rect, radius, pos, true);
		path.moveTo(markerLeft, rect.top() + margin - dist);
		path.lineTo(markerLeft, rect.bottom() - margin + dist);

		painter.setPen(q_parent->palette().color(QPalette::Highlight));
		painter.setBrush(Qt::NoBrush);
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
	FWD(PanelButton, Focused)
#undef GWD
	void PanelButton::setPageFocused(bool value) noexcept { d_func()->setPageFocused(value); }

	QString const& PanelButton::toolTip() const noexcept { return d_func()->toolTip(); }
	void PanelButton::setToolTip(QString const& text) { d_func()->setToolTip(text); }
}  // namespace quick_dra::gui
