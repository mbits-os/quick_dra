// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <qtypes.h>
#include <QPainter>
#include <QVBoxLayout>
#include <vector>
#include "PanelButtonGroup.hpp"

namespace quick_dra::gui {
	class PanelButtonGroup;
	class PanelButton;

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

	namespace PanelButtonStyle {
		static constexpr auto TrueMargin = 18;
		static constexpr auto Margin = TrueMargin + 2;
		static constexpr auto Radius = 6;
		static constexpr auto RadiusF = static_cast<qreal>(Radius);

		struct Palette {
			QColor frame{};
			QColor normal{};
			QColor hover{};
			QColor active{};
			QColor disabled{};
		};

		static constexpr auto lightPalette = Palette{
		    .frame = QColor{230, 228, 230},     // #e6e4e6
		    .normal = QColor{252, 251, 252},    // #fcfbfc
		    .hover = QColor{247, 246, 247},     // #f7f6f7
		    .active = QColor{237, 236, 237},    // #edeced
		    .disabled = QColor{232, 231, 232},  // #e8e7e8
		};

		static constexpr auto darkPalette = Palette{
		    .frame = QColor{30, 29, 30},     // #1e1d1e
		    .normal = QColor{43, 43, 43},    // #2b2b2b
		    .hover = QColor{50, 50, 50},     // #323232
		    .active = QColor{64, 64, 64},    // #404040
		    .disabled = QColor{71, 71, 71},  // #474747
		};
	}  // namespace PanelButtonStyle

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

		void paint(QPainter& painter, Positions pos, PanelButtonStyle::Palette const& palette) const;

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

	class PanelButtonGroupPrivate {
		inline PanelButtonGroup* q_func() noexcept { return static_cast<PanelButtonGroup*>(q_ptr); }
		inline const PanelButtonGroup* q_func() const noexcept { return static_cast<const PanelButtonGroup*>(q_ptr); }
		friend class PanelButtonGroup;
		Q_DISABLE_COPY_MOVE(PanelButtonGroupPrivate)

		Q_GADGET

	public:
		PanelButtonGroupPrivate();
		~PanelButtonGroupPrivate();
		PanelButton* addItem(QLayoutItem*);

		void paintEvent(QPainter& painter, PanelButtonStyle::Palette const& palette);
		void mouseMoveEvent(QPointF const&, Qt::MouseButtons);
		void mousePressEvent(QPointF const&, Qt::MouseButtons);
		void mouseReleaseEvent(QPointF const&, Qt::MouseButtons);

		bool trackingActive() const { return !!originalActive_; }
		PanelButton* fromPos(QPoint const&) const;
		void trackHover(PanelButton*);

	private:
		struct UI {
			void setupUI(PanelButtonGroup*);

			QVBoxLayout* layout{};
		};

		PanelButtonGroup* q_ptr{};
		UI ui{};
		std::vector<std::unique_ptr<PanelButton>> controls_{};
		PanelButton* hovered_{};
		PanelButton* originalActive_{};
	};
}  // namespace quick_dra::gui
