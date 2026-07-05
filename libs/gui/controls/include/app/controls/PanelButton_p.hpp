// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <app/controls/PanelButtonStyle.hpp>
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

	class PanelButtonPrivate {
		inline PanelButton* q_func() noexcept { return static_cast<PanelButton*>(q_ptr); }
		inline const PanelButton* q_func() const noexcept { return static_cast<const PanelButton*>(q_ptr); }
		friend class PanelButton;
		friend class PanelButtonGroup;
		friend class PanelButtonGroupPrivate;

		Q_GADGET

	public:
		~PanelButtonPrivate();

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

		void paint(QPainter& painter,
		           DevicePixelScale const& scale,
		           Positions pos,
		           PanelButtonStyle::Palette const& palette) const;

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

		PanelButton* q_ptr{};
		PanelButtonGroup* q_parent{};
		QLayoutItem* item{};
		bool has_item_ownership : 1 = false;
		bool clickable_ : 1 = false;
		bool enabled_ : 1 = true;
		bool hovered_ : 1 = false;
		bool active_ : 1 = false;
	};
}  // namespace quick_dra::gui
