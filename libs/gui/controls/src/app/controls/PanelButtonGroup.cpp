// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <app/controls/Glyph.hpp>
#include <app/controls/PanelButtonGroup.hpp>
#include <app/controls/PanelButtonGroup_p.hpp>
#include <app/gui/CurrentColor.hpp>
#include <app/utils/LaidOut.hpp>
#include <app/utils/utils.hpp>
#include <memory>
#include <print>
#include <utility>

namespace quick_dra::gui {
	namespace {
		bool lightModeActive(QPalette const& palette) {
			return palette.color(QPalette::Window).toHsl().lightness() >
			       palette.color(QPalette::WindowText).toHsl().lightness();
		}

		PanelButtonStyle::Palette const& selectPalette(QWidget* widget) {
			return lightModeActive(widget->palette()) ? PanelButtonStyle::lightPalette : PanelButtonStyle::darkPalette;
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

	void PanelButtonPrivate::paint(QPainter& painter, Positions pos, PanelButtonStyle::Palette const& palette) const {
		using namespace PanelButtonStyle;
		static constexpr auto DiameterF = RadiusF * 2;

		auto const rect = item->geometry().marginsAdded({TrueMargin, TrueMargin, TrueMargin, TrueMargin}).toRectF();
		QPainterPath path{};
		if (pos & PanePosition::Bottom) {
			path.moveTo(rect.left(), rect.bottom() - RadiusF);
		} else {
			path.moveTo(rect.left(), rect.bottom());
		}

		if (pos & PanePosition::Top) {
			// RIGHT
			path.lineTo(rect.left(), rect.top() + RadiusF);
			// TL corner
			path.arcTo(rect.left(), rect.top(), DiameterF, DiameterF, 180, -90);
			// TOP
			path.lineTo(rect.right() - RadiusF, rect.top());
			// TR corner
			path.arcTo(rect.right() - DiameterF, rect.top(), DiameterF, DiameterF, 90, -90);
		} else {
			// RIGHT
			path.lineTo(rect.left(), rect.top());
			// TOP
			path.lineTo(rect.right(), rect.top());
		}

		if (pos & PanePosition::Bottom) {
			path.lineTo(rect.right(), rect.bottom() - RadiusF);
			path.arcTo(rect.right() - DiameterF, rect.bottom() - DiameterF, DiameterF, DiameterF, 0, -90);
			path.lineTo(rect.left() + RadiusF, rect.bottom());
			path.arcTo(rect.left(), rect.bottom() - DiameterF, DiameterF, DiameterF, 270, -90);
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

	void PanelButtonGroupPrivate::UI::setupUI(PanelButtonGroup* parent) {
		LaidOut{parent}.createLayout(layout, "layout", parent, [](auto& layout) {
			using PanelButtonStyle::Margin;

			layout.setContentsMargins(Margin, Margin, Margin, Margin);
			layout.setSpacing(2 * PanelButtonStyle::TrueMargin - 1);  // only one separating line...
		});
	}

	PanelButtonGroupPrivate::PanelButtonGroupPrivate() = default;
	PanelButtonGroupPrivate::~PanelButtonGroupPrivate() = default;

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

			d.paint(painter, pos, palette);
		}
	}

	void PanelButtonGroupPrivate::mouseMoveEvent(QPointF const& pos) { trackHover(fromPos(pos.toPoint())); }

	void PanelButtonGroupPrivate::mousePressEvent(QPointF const& pos, Qt::MouseButton button) {
		if (button != Qt::MouseButton::LeftButton) {
			return;
		}

		trackHover(fromPos(pos.toPoint()));

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

	void PanelButtonGroupPrivate::mouseReleaseEvent(QPointF const& pos, Qt::MouseButton button) {
		if (button != Qt::MouseButton::LeftButton) {
			return;
		}

		trackHover(fromPos(pos.toPoint()));

		if (!originalActive_) {
			return;
		}

		originalActive_->setActive(false);
		if (hovered_) hovered_->setHovered(true);
		if (originalActive_ == hovered_ && originalActive_->isEnabled() && originalActive_->isClickable()) {
			originalActive_->clicked();
		}
		originalActive_ = nullptr;
		if (!q_func()->geometry().contains(pos.toPoint())) {
			q_func()->setMouseTracking(false);
		}
	}

	PanelButton* PanelButtonGroupPrivate::fromPos(QPoint const& pos) const {
		using namespace PanelButtonStyle;

		for (auto const& control : controls_) {
			auto const& d = *control.get()->d_func();
			if (auto const wdg = d.item->widget(); wdg && wdg->isHidden()) {
				continue;
			}

			auto const rect = d.item->geometry().marginsAdded({TrueMargin, TrueMargin, TrueMargin, TrueMargin});
			if (rect.contains(pos)) {
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

	PanelButtonGroup::PanelButtonGroup(QWidget* parent) : QWidget{parent}, d_ptr{new PanelButtonGroupPrivate} {
		Q_D(PanelButtonGroup);
		d->q_ptr = this;
		d->ui.setupUI(this);
	}

	PanelButtonGroup::~PanelButtonGroup() = default;

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
		painter.scale(1, 1);
		d->paintEvent(painter, selectPalette(this));
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

	Panel::Panel(QWidget* parent) : QWidget{parent} {}

	void setBg(auto* widget, QColor baseColor, int alpha = 20) {
		auto pal = widget->palette();
		baseColor.setAlpha(alpha);
		pal.setColor(QPalette::Window, baseColor);
		widget->setPalette(pal);
		widget->setAutoFillBackground(true);
	}

	void Panel::setInfo(QString const& label, QString const& details, QString const& value, QIcon const& rightIcon) {
		if (layout()) {
			QWidget{}.setLayout(layout());
		}

		// +-HBox----------------------------------------+--+-------------+
		// |+-VBox--------------------------------------+|  |             |
		// ||+-HBox-----------------+------------------+||  |             |
		// |||[label] TakeWidth/1_x |[value] TakeWidth |||  | TakeHeight  |
		// ||+----------------------+------------------+||  | [rightIcon] |
		// |+-------------------------------------------+|  |             |
		// ||[details] TakeWidth                        ||  |             |
		// |+-------------------------------------------+|  |             |
		// +---------------------------------------------+--+-------------+

		// +-HBox------------------------------------+--+-----------------------+
		// |[label] TakeWidth/1_x |[value] TakeWidth |  |[rightIcon] TakeHeight |
		// +-----------------------------------------+--+-----------------------+

		QHBoxLayout* labelAndValue{};

		auto const tight = [](auto& layout) {
			layout.setContentsMargins(0, 0, 0, 0);
			layout.setSpacing(0);
		};

		LaidOut{this}.createLayout(root_, "rootLayout", this, tight);
		auto root = LaidOut{this, root_};

		if (details.isEmpty()) {
			labelAndValue = root_;
		} else {
			QVBoxLayout* leftHandSide{};
			QLabel* detailsLabel{};

			root.createLayout(leftHandSide, "leftLayout", tight);

			LaidOut{this, leftHandSide}
			    .createLayout(labelAndValue, "labelLayout", tight)
			    .createWidget(detailsLabel, "details", [&details](QLabel& lbl) {
				    lbl.setSizePolicy(TakeWidth / HeightForWidth);
				    lbl.setText(details);
				    lbl.setTextFormat(Qt::MarkdownText);
				    lbl.setWordWrap(true);

				    auto font = lbl.font();
				    font.setPointSizeF(font.pointSizeF() * .85);
				    lbl.setFont(font);
			    });
		}

		auto horiz = LaidOut{this, labelAndValue};
		horiz.addWidget<QLabel>("label", [&label](QLabel& lbl) {
			lbl.setSizePolicy(TakeWidth / (HeightForWidth / 1_XStretch));
			lbl.setText(label);
			lbl.setTextFormat(Qt::MarkdownText);
			lbl.setWordWrap(true);
			lbl.setAlignment(Qt::AlignTop | Qt::AlignLeft);

			auto font = lbl.font();
			font.setWeight(QFont::ExtraBold);
			lbl.setFont(font);
		});
		value_ = horiz.addWidget<QLabel>("value", [value](QLabel& lbl) {
			lbl.setSizePolicy(TakeWidth);
			lbl.setText(value);
			lbl.setAlignment(Qt::AlignTop | Qt::AlignRight);
		});

		root_->addSpacing(10);

		root.addWidget<Glyph>("", [&rightIcon](Glyph& glyph) {
			glyph.setSizePolicy(TakeHeight);
			glyph.setIcon(rightIcon.isNull() ? nullSVGIcon() : rightIcon);
			glyph.setIconSize(24, 24);
		});
	}

}  // namespace quick_dra::gui
