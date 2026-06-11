// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QMainWindow>

#include <QDebug>
#include <QPainter>
#include <QTest>
#include <app/gui/CurrentColor.hpp>
#include <app/utils/utils.hpp>
#include <array>
#include <format>
#include <quick_dra/base/str.hpp>
#include "GuiTest.hpp"
#include "palette_override.hpp"
#include "stamper.hpp"

using namespace quick_dra::gui;
using namespace quick_dra;

static constexpr auto iconCreators = std::array{
    gui::arrowRightSVGIcon, gui::arrowLeftSVGIcon, gui::ellipsisSVGIcon, gui::checkSVGIcon,
    gui::nullSVGIcon,       gui::resetSVGIcon,     gui::warningSVGIcon,
};

class CurrentColorWidget : public QWidget {
public:
	CurrentColorWidget() {
		auto it = icons.begin();
		for (auto const& create : iconCreators) {
			*it++ = create();
		}
		int width = 0;
		int height = 2 * margin;

		for (auto const& icon : icons) {
			auto const size = icon.actualSize({16, 16});
			width += margin + size.width();
			auto const h = 2 * margin + size.height();
			height = std::max(h, height);
		}
		width += margin;
		resize(std::max(width, 2 * margin), height);
	}

	void paintEvent(QPaintEvent*) override {
		QPainter painter{this};
		auto const h = height();
		auto x = margin;
		for (auto const& icon : icons) {
			auto const size = icon.actualSize({16, 16});
			auto const y = (h - size.height()) / 2;

			icon.paint(&painter, QRect{x, y, size.width(), size.height()}, Qt::AlignCenter,
			           isEnabled() ? QIcon::Active : QIcon::Disabled);

			x += margin + size.width();
		}
	}

	template <size_t Index>
	QIcon copy() const {
		if constexpr (Index <= iconCreators.size()) {
			return icons[Index];
		}
	}

private:
	static constexpr auto margin = 6;
	std::array<QIcon, iconCreators.size()> icons{};
};

static PaletteOverride const tests[] = {
    {.window = Qt::lightGray, .windowText = Qt::black},
    {.window = Qt::black, .windowText = Qt::white},
    {.window = QColor{40, 0, 0}, .windowText = QColor{255, 200, 0, 192}},
};

void GuiTest::CurrentColor_changePaletteAtRuntime() {
	CurrentColorWidget widget{};

	Stamper stamper{
	    &widget,
	    {
	        .path = "images/CurrentColor.png"sv,
	        .columns = 2,
	        .rows = static_cast<int>(std::size(tests)),
	    },
	};

	for (auto const& pos : stamper) {
		auto const enabled = pos.column == 0;
		auto const& test = tests[pos.row];
		if (enabled) {
			// disabled will come next
			test.install(&widget);
		}
		widget.setEnabled(enabled);
		stamper.grab(pos);
	}

	auto stencil = stamper.loadStencil();
	COMPARE_IMAGES(stencil, stamper);
}
