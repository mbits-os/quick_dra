// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <app/controls/Glyph.hpp>
#include <app/controls/Panel.hpp>
#include <app/controls/PanelButtonGroup.hpp>
#include <app/controls/PanelButtonGroup_p.hpp>
#include <app/gui/CurrentColor.hpp>
#include <app/utils/LaidOut.hpp>
#include <app/utils/utils.hpp>
#include <memory>
#include <utility>

namespace quick_dra::gui {
	namespace {
		bool lightModeActive(QPalette const& palette) {
			return palette.color(QPalette::Window).toHsl().lightness() >
			       palette.color(QPalette::WindowText).toHsl().lightness();
		}

		PanelButtonStyle::Palette const& selectPalette() {
			return lightModeActive(qApp->palette()) ? PanelButtonStyle::lightPalette : PanelButtonStyle::darkPalette;
		}

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

	void PanelButtonGroupPrivate::UI::setupUI(DevicePixelScale const& scale, PanelButtonGroup* parent) {
		LaidOut{parent}.createLayout(layout, "layout", parent);
		setMargins(scale);
	}

	void PanelButtonGroupPrivate::UI::setMargins(DevicePixelScale const& scale) {
		auto const spacing = 2 * PanelButtonStyle::TrueMargin - 1_px;  // only one separating line...
		auto const margin = scale.toDevice(PanelButtonStyle::Margin);
		layout->setContentsMargins(margin, margin, margin, margin);
		layout->setSpacing(scale.toDevice(spacing));
	}

	PanelButtonGroupPrivate::PanelButtonGroupPrivate(PanelButtonGroup* q_ptr) : q_ptr{q_ptr} {
		ui.setupUI(scale, q_ptr);
	}

	PanelButtonGroupPrivate::~PanelButtonGroupPrivate() = default;

	int PanelButtonGroupPrivate::count() const { return static_cast<int>(controls_.size()); }
	PanelButton* PanelButtonGroupPrivate::itemAt(int pos) const { return controls_.at(static_cast<size_t>(pos)).get(); }
	PanelButton* PanelButtonGroupPrivate::addItem(QLayoutItem* item) {
		auto priv = std::make_unique<PanelButton>();
		auto result = priv.get();

		auto const d = priv->d_func();
		d->item = item;
		d->q_parent = q_func();
		controls_.emplace_back(std::move(priv));

		return result;
	}

	void PanelButtonGroupPrivate::paintEvent(QPainter& painter, PanelButtonStyle::Palette const& palette) {
		// GCOV_EXCL_START
		if (scale.updateScale(q_ptr->logicalDpiX())) {
			[[unlikely]];
			ui.setMargins(scale);
		}
		// GCOV_EXCL_STOP

		auto lastIndex = -1;

		for (auto index = -1; auto const& control : controls_) {
			++index;
			auto const& d = *control.get()->d_func();
			if (auto const wdg = d.item->widget(); wdg && wdg->isHidden()) {
				continue;
			}
			lastIndex = index;
		}

		auto first = true;
		for (auto index = -1; auto const& control : controls_) {
			++index;
			auto const& d = *control.get()->d_func();
			if (auto const wdg = d.item->widget(); wdg && wdg->isHidden()) {
				continue;
			}

			Positions pos{};
			if (first) pos |= PanePosition::Top;
			if (index == lastIndex) pos |= PanePosition::Bottom;
			first = false;

			d.paint(painter, scale, pos, palette);
		}
	}

	void PanelButtonGroupPrivate::mouseMoveEvent(QPointF const& inWidgetPos) {
		trackHover(fromPos(inWidgetPos.toPoint()));
	}

	void PanelButtonGroupPrivate::mousePressEvent(QPointF const& inWidgetPos, Qt::MouseButton button) {
		if (button != Qt::MouseButton::LeftButton) {
			return;
		}

		trackHover(fromPos(inWidgetPos.toPoint()));

		if (hovered_ && hovered_->isClickable()) {
			originalActive_ = hovered_;
		} else {
			originalActive_ = nullptr;
		}
		if (originalActive_) {
			originalActive_->setHovered(false);
			originalActive_->setActive(true);
		}
	}

	void PanelButtonGroupPrivate::mouseReleaseEvent(QPointF const& inWidgetPos, Qt::MouseButton button) {
		if (button != Qt::MouseButton::LeftButton) {
			return;
		}

		trackHover(fromPos(inWidgetPos.toPoint()));

		if (!originalActive_) {
			return;
		}

		originalActive_->setActive(false);
		if (hovered_) hovered_->setHovered(true);
		if (originalActive_ == hovered_ && originalActive_->isEnabled() && originalActive_->isClickable()) {
			originalActive_->clicked();
		}
		originalActive_ = nullptr;
		auto const rect = q_func()->rect().toRectF();  // scale.fromDevice(q_func()->rect());
		qDebug() << "mouseRelease" << inWidgetPos << q_func()->rect() << rect.contains(inWidgetPos);
		if (!rect.contains(inWidgetPos)) {
			q_func()->setMouseTracking(false);
		}
	}

	PanelButton* PanelButtonGroupPrivate::fromPos(QPoint const& inWidgetPos) {
		// GCOV_EXCL_START
		if (scale.updateScale(q_ptr->logicalDpiX())) {
			[[unlikely]];
			ui.setMargins(scale);
		}
		// GCOV_EXCL_STOP

		auto const trueMargin = scale.toDevice(PanelButtonStyle::TrueMargin);

		for (auto const& control : controls_) {
			auto const& d = *control.get()->d_func();
			if (auto const wdg = d.item->widget(); wdg && wdg->isHidden()) {
				continue;
			}

			auto const rect = d.item->geometry().marginsAdded({trueMargin, trueMargin, trueMargin, trueMargin});
			if (rect.contains(inWidgetPos)) {
				return control.get();
			}
		}

		return nullptr;
	}

	void PanelButtonGroupPrivate::trackHover(PanelButton* next) {
		if (next == hovered_) return;
		if (hovered_) hovered_->setHovered(false);
		hovered_ = next;
		if (originalActive_) {
			originalActive_->setActive(originalActive_ == hovered_);
		} else {
			if (hovered_) hovered_->setHovered(true);
		}

		if (hovered_) {
			q_func()->setCursor(hovered_->d_func()->cursor());
		} else {
			q_func()->setCursor(Qt::ArrowCursor);
		}
	}

	PanelButtonGroup::PanelButtonGroup(QWidget* parent) : QWidget{parent}, d_ptr{new PanelButtonGroupPrivate{this}} {}

	PanelButtonGroup::~PanelButtonGroup() = default;

	int PanelButtonGroup::count() const { return d_func()->count(); }

	PanelButton* PanelButtonGroup::itemAt(int pos) const { return d_func()->itemAt(pos); }

	PanelButton* PanelButtonGroup::addButton(QString const& label, bool bold) {
		return createWidget<QLabel>([&label, bold](QLabel& control) {
			control.setText(label);
			control.setSizePolicy(TakeWidth / HeightForWidth);
			control.setWordWrap(true);
			if (bold) {
				auto font = control.font();
				font.setBold(true);
				control.setFont(font);
			}
		});
	}

	PanelButton* PanelButtonGroup::addWidget(QWidget* widget) {
		Q_D(PanelButtonGroup);
		auto const count = d->ui.layout->count();
		d->ui.layout->addWidget(widget);
		auto item = d->ui.layout->itemAt(count);
		return d->addItem(item);
	}

	PanelButton* PanelButtonGroup::addLayout(QLayout* layout) {
		Q_D(PanelButtonGroup);
		auto const count = d->ui.layout->count();
		d->ui.layout->addLayout(layout);
		auto item = d->ui.layout->itemAt(count);
		return d->addItem(item);
	}

	PanelButton* PanelButtonGroup::createPanel(PanelInfo const& info, QAnyStringView objectName) {
		auto const result = createWidget<Panel>(objectName, [&info](Panel& panel) { panel.setInfo(info); });
		if (info.isClickable) {
			result->setClickable(*info.isClickable);
		}
		if (info.isEnabled) {
			result->setEnabled(*info.isEnabled);
		}
		return result;
	}

	PanelButton* PanelButtonGroup::takeLast() {
		Q_D(PanelButtonGroup);
		if (d->controls_.empty()) {
			return nullptr;
		}

		auto local = std::move(d->controls_.back());
		d->controls_.pop_back();

		auto button_d = local->d_func();

		if (local) {
			auto const index = d->ui.layout->indexOf(button_d->item);
			d->ui.layout->takeAt(index);
			button_d->has_item_ownership = true;
		}
		return local.release();
	}

	void PanelButtonGroup::clearAll() {
		while (auto item = takeLast()) {
			item->clearItem();
			delete item;
		}
	}

	void PanelButtonGroup::paintEvent(QPaintEvent* event) {
		QWidget::paintEvent(event);

		Q_D(PanelButtonGroup);

		QPainter painter{this};
		painter.setRenderHint(QPainter::Antialiasing);
		d->paintEvent(painter, selectPalette());
	}

	void PanelButtonGroup::enterEvent(QEnterEvent* event) {
		Q_D(PanelButtonGroup);

		setMouseTracking(true);

		d->mouseMoveEvent(event->position());
	}

	void PanelButtonGroup::leaveEvent(QEvent*) {
		Q_D(PanelButtonGroup);

		if (!d->trackingActive()) setMouseTracking(false);
		d->trackHover(nullptr);
	}

	void PanelButtonGroup::mouseMoveEvent(QMouseEvent* event) {
		Q_D(PanelButtonGroup);

		d->mouseMoveEvent(event->position());
	}
	void PanelButtonGroup::mousePressEvent(QMouseEvent* event) {
		Q_D(PanelButtonGroup);

		d->mousePressEvent(event->position(), event->button());
	}
	void PanelButtonGroup::mouseReleaseEvent(QMouseEvent* event) {
		Q_D(PanelButtonGroup);

		d->mouseReleaseEvent(event->position(), event->button());
	}
}  // namespace quick_dra::gui
