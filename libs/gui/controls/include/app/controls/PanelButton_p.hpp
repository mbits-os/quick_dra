// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QShortcut>
#include <app/controls/PanelButtonStyle.hpp>
#include <vector>
#include "PanelButton.hpp"

namespace quick_dra::gui {
	enum class PanePosition {
		Middle = 0,
		Top = 1,
		Bottom = 2,
		Both = Top | Bottom,
	};

	Q_DECLARE_FLAGS(Positions, PanePosition)
	Q_DECLARE_OPERATORS_FOR_FLAGS(Positions)

	enum class PaneState {
		Normal,
		Hovered,
		Pressed,
	};

	class PanelButtonPrivate;
	struct Shortcuts : QObject {
		Q_OBJECT

	public:
		explicit Shortcuts(PanelButtonPrivate* parent);

		void setEnabled(bool value);
		void setFocused(bool value);
		void setSequences(QList<QKeySequence> const&);

	private slots:
		void activated();
		void activatedAmbiguously();

	private:
		void enableShortcuts(bool enabledAndFocused);

		QList<QKeySequence> sequences{};
		std::vector<QShortcut*> shortcuts{};
		bool enabled : 1 = true;
		bool focused : 1 = true;
	};

	class PanelButtonPrivate : public QObject {
		friend class PanelButton;
		friend class PanelButtonGroup;
		friend class PanelButtonGroupPrivate;

		Q_OBJECT

	public:
		~PanelButtonPrivate();

		inline PanelButton* q_func() noexcept { return static_cast<PanelButton*>(q_ptr); }
		inline const PanelButton* q_func() const noexcept { return static_cast<const PanelButton*>(q_ptr); }

		void setSequences(QList<QKeySequence> const&);
		QString const& toolTip() const noexcept { return toolTip_; }
		void setToolTip(QString const&);
		QRect geometry() const;
		void ensureVisible() const;

		auto cursor() const noexcept { return clickable_ && enabled_ ? Qt::PointingHandCursor : Qt::ArrowCursor; }

		bool isClickable() const noexcept { return clickable_; }
		void setClickable(bool value) noexcept {
			if (value == clickable_) return;
			clickable_ = value;
			q_parent->update();
		}

		bool isEnabled() const noexcept { return enabled_; }
		void setEnabled(bool value) noexcept {
			propagate(item, [value](QWidget* wgt) { wgt->setEnabled(value); });
			if (value == enabled_) return;
			enabled_ = value;
			shortcuts->setEnabled(value);
			q_parent->update();
		}

		bool isHovered() const noexcept { return hovered_; }
		void setHovered(bool value) noexcept {
			if (value == hovered_) return;
			hovered_ = value;
			q_parent->update();
		}

		bool isActive() const noexcept { return active_; }
		void setActive(bool value) noexcept {
			if (value == active_) return;
			active_ = value;
			q_parent->update();
		}

		bool isFocused() const noexcept { return focused_; }
		void setFocused(bool value) noexcept {
			if (value == focused_) return;
			focused_ = value;
			if (focused_) ensureVisible();
			q_parent->update();
		}

		void setPageFocused(bool value) noexcept { shortcuts->setFocused(value); }

		void paint(QPainter& painter,
		           DevicePixelScale const& scale,
		           Positions pos,
		           PanelButtonStyle::Palette const& palette) const;
		void paintFocusRect(QPainter& painter, DevicePixelScale const& scale, Positions pos) const;

	private:
		void propagate(QLayoutItem* item, auto&& callback) {
			if (auto widget = item->widget()) {
				callback(widget);
			} else if (auto layout = item->layout()) {
				auto const count = layout->count();
				for (auto index = 0; index < count; ++index) {
					auto child = layout->itemAt(index);
					propagate(child, callback);
				}
			}
		}

		void updateToolTip();

		PanelButton* q_ptr{};
		PanelButtonGroup* q_parent{};
		QLayoutItem* item{};
		Shortcuts* shortcuts{new Shortcuts{this}};
		QString buttonTip{};
		QString keyTip{};
		QString toolTip_{};
		bool has_item_ownership : 1 = false;
		bool clickable_ : 1 = false;
		bool enabled_ : 1 = true;
		bool hovered_ : 1 = false;
		bool active_ : 1 = false;
		bool focused_ : 1 = false;
	};
}  // namespace quick_dra::gui
