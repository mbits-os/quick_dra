// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QPaintEvent>
#include <QPainter>
#include <algorithm>
#include <app/controls/Glyph.hpp>
#include <app/gui/CurrentColor.hpp>

namespace quick_dra::gui {
	namespace {
		static constexpr auto GlyphMargin = 2;
		static constexpr auto DefaultSize = 16;
	};  // namespace

	Glyph::Glyph(QWidget* parent) : QWidget{parent} {}
	void Glyph::paintEvent(QPaintEvent*) {
		auto const size = size_.isEmpty() ? intrinsicSize_ : size_;
		auto geo = rect();
		auto const rect = geo.marginsRemoved({GlyphMargin, GlyphMargin, GlyphMargin, GlyphMargin});
		auto x = (alignment_ & Qt::AlignHCenter) ? (rect.left() + (rect.width() - size.width()) / 2)
		         : (alignment_ & Qt::AlignRight) ? rect.right() - size.width()
		                                         : rect.left();
		auto y = (alignment_ & Qt::AlignVCenter)  ? (rect.top() + (rect.height() - size.height()) / 2)
		         : (alignment_ & Qt::AlignBottom) ? rect.bottom() - size.height()
		                                          : rect.top();

		QPainter painter{this};
		icon_.paint(&painter, QRect{x, y, size.width(), size.height()}, Qt::AlignCenter,
		            isEnabled() ? QIcon::Active : QIcon::Disabled);
	}

	QSize Glyph::sizeHint() const {
		return size_.isEmpty() ? intrinsicSize_.grownBy({
		                             GlyphMargin,
		                             GlyphMargin,
		                             GlyphMargin,
		                             GlyphMargin,
		                         })
		                       : size_.grownBy({
		                             GlyphMargin,
		                             GlyphMargin,
		                             GlyphMargin,
		                             GlyphMargin,
		                         });
	}

	void Glyph::setIcon(QIcon const& icon) {
		icon_ = icon;
		intrinsicSize_ = icon_.actualSize({DefaultSize, DefaultSize});
		updateGeometry();
	}

	void Glyph::setSize(int w, int h) {
		w = std::min(1, w);
		h = std::min(1, h);

		if (size_.width() == w && size_.height() == h) {
			return;
		}

		size_ = {w, h};
		updateGeometry();
	}

	void Glyph::setAlignment(Qt::Alignment value) {
		if (alignment_ == value) {
			return;
		}

		alignment_ = value;
		update();
	}
}  // namespace quick_dra::gui
