// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <algorithm>
#include <app/controls/DevicePixelScale.hpp>
#include <app/controls/WordWrap.hpp>
#include "Panel.hpp"

namespace quick_dra::gui {
	class PanelPrivate {
	public:
		static constexpr auto TEXT_MARGIN_PIXELS = 4_px;
		static constexpr auto ICON_MARGIN_PIXELS = 6_px;
		static constexpr auto ICON_SIZE_PIXELS = 16_px;
		static constexpr auto DETAIL_FONT_SIZE = qreal{.85};

		PanelPrivate(Panel* q) : q_ptr{q} {
			ui.title.setIsHeading(true);
			ui.details.setFontSize(DETAIL_FONT_SIZE);
		}

		void setTitle(QString const&);
		void setDetails(QString const&);
		void setValue(QString const&);
		void setIcon(QIcon const&);
		void updateFont();
		void setWidth(int width);
		void requestLayout();

		int heightForWidth(int);
		QSize const& sizeHint();
		QSize const& minimumSizeHint();
		void layout();
		void paint(QPainter*);

	private:
		void requestOptimalSizes();
		void doLayout();
		void paintIcon(QPainter*, int width, int height);

		template <typename Handler>
		auto layoutImpl(qreal requestedWidth, Handler&& handler = {}) {
			ui.title.parse();
			ui.details.parse();

			auto const ICON_SIZE = scale.toDeviceF(ICON_SIZE_PIXELS);
			auto const ICON_MARGIN = scale.toDeviceF(ICON_MARGIN_PIXELS);
			auto const TEXT_MARGIN = scale.toDeviceF(TEXT_MARGIN_PIXELS);

			auto const hasTitle = !ui.title.cleanText().isEmpty();
			auto const hasValue = !ui.value.text.isEmpty();
			auto const hasDetails = !ui.details.cleanText().isEmpty();

			auto leftColumnHeight = qreal{0};
			auto availableWidth = requestedWidth - ICON_MARGIN - ICON_SIZE;
			auto titleWidth = availableWidth;

			if (hasValue) {
				leftColumnHeight = ui.value.boundingBox.height();
				titleWidth -= ui.value.boundingBox.width() + TEXT_MARGIN;
				handler.moveValue(titleWidth + TEXT_MARGIN, 0, ui.value);
			}

			if (hasTitle) {
				auto const titleHeight = handler.labelHeight(0, 0, titleWidth, ui.title);
				leftColumnHeight = std::max(titleHeight, leftColumnHeight);
			}

			if (hasDetails) {
				if (hasTitle || hasValue) {
					leftColumnHeight += TEXT_MARGIN;
				}

				auto const detailsHeight = handler.labelHeight(0, leftColumnHeight, availableWidth, ui.details);
				leftColumnHeight += detailsHeight;
			}

			auto const height = std::max(leftColumnHeight, ICON_SIZE);
			if (height != leftColumnHeight) {
				auto const adjustment = (height - leftColumnHeight) / 2;
				if (hasTitle) handler.adjustHeightChange(adjustment, ui.title);
				if (hasValue) handler.adjustHeightChange(adjustment, ui.value);
				if (hasDetails) handler.adjustHeightChange(adjustment, ui.details);
			}
			return height;
		}

		template <typename Sizer>
		auto calcOptimalSize(Sizer&& sizer = {}) {
			ui.title.parse();
			ui.details.parse();

			auto const ICON_SIZE = scale.toDeviceF(ICON_SIZE_PIXELS);
			auto const ICON_MARGIN = scale.toDeviceF(ICON_MARGIN_PIXELS);
			auto const TEXT_MARGIN = scale.toDeviceF(TEXT_MARGIN_PIXELS);

			auto const hasValue = !ui.value.text.isEmpty();
			auto const hasTitle = !ui.title.cleanText().isEmpty();
			auto const hasDetails = !ui.details.cleanText().isEmpty();

			auto totalHeight = qreal{0};
			auto totalWidth = qreal{0};

			if (hasValue) {
				totalWidth = ui.value.boundingBox.width();
				totalHeight = ui.value.boundingBox.height();

				if (hasTitle) {
					totalWidth += TEXT_MARGIN;
				}
			}

			if (hasTitle) {
				auto const titleSize = sizer.sizeFrom(ui.title);
				totalWidth += titleSize.width();
				totalHeight = std::max(titleSize.height(), totalHeight);
			}

			if (hasDetails) {
				if (hasTitle || hasValue) {
					totalHeight += TEXT_MARGIN;
				}

				auto const detailsSize = sizer.sizeFrom(ui.details);
				totalWidth = std::max(totalWidth, detailsSize.width());
				totalHeight += detailsSize.height();
			}

			totalWidth += ICON_MARGIN + ICON_SIZE;
			totalHeight = std::max(totalHeight, ICON_SIZE);

			return QSizeF{totalWidth, totalHeight};
		}

		class HeightForWidthHandler;
		class LayoutHandler;

		class HintSizer;
		class MinimalSizer;

		struct Value {
			QString text{};
			QRectF boundingBox{};
			LineHeight lineHeight{};
			QPointF offset{};

			void moveTo(QPointF const& pos) noexcept { offset = pos; }
			bool setText(QString const& text, QFont const&);
			void measure(QFont const&);
			void paint(QPainter*);
		};

		struct UI {
			MarkdownLabel title{};
			MarkdownLabel details{};
			Value value{};
			QIcon icon{};
		};

		bool needsLayout_{false};
		bool optimalSizesInvalid_{false};
		QSize hintedSize_{};
		QSize minimalSize_{};
		qreal currentWidth_{};

		UI ui{};
		Panel* q_ptr{};
		DevicePixelScale scale{q_ptr->logicalDpiX()};
	};
}  // namespace quick_dra::gui
