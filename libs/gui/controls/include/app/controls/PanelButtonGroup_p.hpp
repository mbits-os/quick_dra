// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QVBoxLayout>
#include <app/controls/PanelButtonStyle.hpp>
#include <app/utils/DevicePixelScale.hpp>
#include <vector>
#include "PanelButtonGroup.hpp"

namespace quick_dra::gui {
	namespace PanelButtonStyle {
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

	class PanelButtonGroupPrivate {
		inline PanelButtonGroup* q_func() noexcept { return static_cast<PanelButtonGroup*>(q_ptr); }
		inline const PanelButtonGroup* q_func() const noexcept { return static_cast<const PanelButtonGroup*>(q_ptr); }
		friend class PanelButtonGroup;
		Q_DISABLE_COPY_MOVE(PanelButtonGroupPrivate)

		Q_GADGET

	public:
		PanelButtonGroupPrivate(PanelButtonGroup* q_ptr);
		~PanelButtonGroupPrivate();

		int count() const;
		PanelButton* itemAt(int) const;
		PanelButton* addItem(QLayoutItem*);

		void paintEvent(QPainter& painter, PanelButtonStyle::Palette const& palette);
		void mouseMoveEvent(QPointF const&);
		bool mousePressEvent(QPointF const&, Qt::MouseButton);
		bool mouseReleaseEvent(QPointF const&, Qt::MouseButton);
		bool toolTipEvent(QPoint const& inWidgetPos, QPoint const& globalPos);
		void pageFocusEvent(bool hasFocus);
		void gotFocus(Qt::FocusReason reason);
		void lostFocus();
		bool focusPrevNext(bool next);

		bool trackingActive() const { return !!originalActive_; }
		PanelButton* fromPos(QPoint const&);
		void trackHover(PanelButton*);

	private:
		struct UI {
			void setupUI(DevicePixelScale const& scale, PanelButtonGroup*);
			void setMargins(DevicePixelScale const& scale);

			QVBoxLayout* layout{};
		};

		PanelButton* prevNext(int diff);
		bool isTabStop(PanelButton*);
		void setFocused(PanelButton*);
		void setInternalFocus(bool value);

		PanelButtonGroup* q_ptr{};
		DevicePixelScale scale{q_ptr->logicalDpiX()};
		UI ui{};
		std::vector<std::unique_ptr<PanelButton>> controls_{};
		bool hasFocus_{false};
		int focusedIndex_{-1};
		PanelButton* focused_{};
		PanelButton* hovered_{};
		PanelButton* originalActive_{};
	};
}  // namespace quick_dra::gui
