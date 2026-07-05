// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <app/controls/PanelButtonGroup.hpp>
#include <app/controls/PanelButtonGroup_p.hpp>

#include <QApplication>
#include <QHelpEvent>
#include <QLabel>
#include <QToolTip>
#include <app/controls/Panel.hpp>
#include <app/controls/PanelButton_p.hpp>
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
	}  // namespace

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
		if (!rect.contains(inWidgetPos)) {
			q_func()->setMouseTracking(false);
		}
	}

	bool PanelButtonGroupPrivate::toolTipEvent(QPoint const& inWidgetPos, QPoint const& globalPos) {
		auto const button = fromPos(inWidgetPos);
		if (button == nullptr || button->toolTip().isEmpty()) {
			QToolTip::hideText();
			return false;
		}

		QToolTip::showText(globalPos, button->toolTip());
		return true;
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

	PanelButton* PanelButtonGroup::createPanel(CreatePanelOptions const& info, QAnyStringView objectName) {
		auto const panelButton = createWidget<Panel>(objectName, [&info](Panel& panel) {
			panel.setInfo({
			    .label = info.label,
			    .details = info.details,
			    .value = info.value,
			    .rightIcon = info.rightIcon,
			});
		});
		if (info.isClickable) {
			panelButton->setClickable(*info.isClickable);
		}
		if (info.isEnabled) {
			panelButton->setEnabled(*info.isEnabled);
		}
		panelButton->setToolTip(info.toolTip);
		panelButton->setSequences(info.sequences);
		return panelButton;
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

	bool PanelButtonGroup::event(QEvent* event) {
		if (event->type() == QEvent::ToolTip) {
			auto helpEvent = static_cast<QHelpEvent*>(event);
			if (!d_ptr->toolTipEvent(helpEvent->pos(), helpEvent->globalPos())) {
				event->ignore();
			}
			return true;
		}
		return QWidget::event(event);
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
