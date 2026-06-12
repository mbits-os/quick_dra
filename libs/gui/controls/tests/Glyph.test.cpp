// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QTest>
#include <app/controls/Glyph.hpp>
#include <app/gui/CurrentColor.hpp>
#include <quick_dra/base/str.hpp>
#include "ControlsTest.hpp"
#include "palette_override.hpp"
#include "stamper.hpp"

using namespace quick_dra::gui;
using namespace quick_dra;

void ControlsTest::glyphProperties() {
	Glyph glyph{};
	QCOMPARE_EQ(glyph.minimumSizeHint(), (QSize{4, 4}));
	glyph.setIcon(resetSVGIcon());
	QCOMPARE_EQ(glyph.minimumSizeHint(), (QSize{20, 20}));
	glyph.setIconSize(30, 30);
	QCOMPARE_EQ(glyph.minimumSizeHint(), (QSize{34, 34}));
	glyph.setIconSize(24, 24);
	QCOMPARE_EQ(glyph.minimumSizeHint(), (QSize{28, 28}));
	glyph.setIconSize(24, 24);  // set the same values, TODO: check side effects of setGeometry
	QCOMPARE_EQ(glyph.minimumSizeHint(), (QSize{28, 28}));

	glyph.setAlignment(Qt::AlignTop | Qt::AlignLeft);
	for (auto const vert : {Qt::AlignTop, Qt::AlignVCenter, Qt::AlignBottom}) {
		for (auto const horiz : {Qt::AlignLeft, Qt::AlignHCenter, Qt::AlignRight}) {
			auto const alignment = vert | horiz;
			glyph.setAlignment(alignment);
			QCOMPARE_EQ(glyph.alignment(), alignment);
		}
	}
}

void ControlsTest::glyphPainter() {
	Glyph glyph{};
	glyph.setIcon(resetSVGIcon());
	glyph.resize(30, 30);
	auto const colors = PaletteOverride{.window = QColor{33, 33, 33, 255}, .windowText = Qt::white};
	colors.install(&glyph);

	auto const verts = std::array{Qt::AlignTop, Qt::AlignVCenter, Qt::AlignBottom};
	auto const horizes = std::array{Qt::AlignLeft, Qt::AlignHCenter, Qt::AlignRight};

	Stamper stamper{
	    &glyph,
	    {
	        .path = "images/GlyphAlignment.png"sv,
	        .columns = static_cast<int>(horizes.size()),
	        .rows = static_cast<int>(verts.size()),
	        .margins = {1, 1, 1, 1},
	        .spacing = 1,
	        .background = QColor{17, 17, 17, 255},
	    },
	};

	for (auto const& pos : stamper) {
		auto const row = safe_size_t(pos.row);
		auto const column = safe_size_t(pos.column);
		glyph.setAlignment(verts[row] | horizes[column]);
		stamper.grab(pos);
	}

	auto stencil = stamper.loadStencil();
	COMPARE_IMAGES(stencil, stamper);
}

void ControlsTest::glyphSize() {
	Glyph glyph{};
	glyph.setIcon(resetSVGIcon());
	glyph.resize(32, 32);
	auto const colors = PaletteOverride{.window = QColor{240, 240, 240, 255}, .windowText = Qt::black};
	colors.install(&glyph);

	auto const sizes = std::array{13, 16, 19, 22, 26, 29, 32};

	Stamper stamper{
	    &glyph,
	    {
	        .path = "images/GlyphIconSize.png"sv,
	        .columns = static_cast<int>(sizes.size()),
	        .rows = 1,
	        .margins = {1, 1, 1, 1},
	        .spacing = 1,
	        .background = QColor{17, 17, 17, 255},
	    },
	};

	for (auto const& pos : stamper) {
		auto const size = sizes[safe_size_t(pos.column)];
		glyph.setIconSize(size, size);
		stamper.grab(pos);
	}

	auto stencil = stamper.loadStencil();
	COMPARE_IMAGES(stencil, stamper);
}
