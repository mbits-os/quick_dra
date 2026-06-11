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
	CurrentColorWidget(int iconWidth, int iconHeight = -1)
	    : iconWidth_{iconWidth}, iconHeight_{iconHeight < 1 ? iconWidth : iconHeight} {
		auto it = icons.begin();
		for (auto const& create : iconCreators) {
			*it++ = create();
		}

		resize(margin + (iconWidth + margin) * static_cast<int>(icons.size()), iconHeight + 2 * margin);
	}

	void paintEvent(QPaintEvent*) override {
		QPainter painter{this};
		auto const y = margin;
		auto x = margin;
		for (auto const& icon : icons) {
			icon.paint(&painter, QRect{x, y, iconWidth_, iconHeight_}, Qt::AlignCenter,
			           isEnabled() ? QIcon::Active : QIcon::Disabled);

			x += margin + iconWidth_;
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
	int iconWidth_;
	int iconHeight_;
	std::array<QIcon, iconCreators.size()> icons{};
};

static PaletteOverride const tests[] = {
    {.window = Qt::lightGray, .windowText = Qt::black},
    {.window = Qt::black, .windowText = Qt::white},
    {.window = QColor{40, 0, 0}, .windowText = QColor{255, 200, 0, 192}},
};

void GuiTest::CurrentColor_changePaletteAtRuntime_data() {
	QTest::addColumn<int>("iconWidth");
	QTest::addColumn<int>("iconHeight");
	for (auto const size : {12, 16, 20, 24, 32}) {
		QTest::newRow(std::format("{}px", size).c_str()) << size << size;
	}

	QTest::newRow("narrow") << 12 << 24;
	QTest::newRow("wide") << 32 << 16;
}

void GuiTest::CurrentColor_changePaletteAtRuntime() {
	QFETCH(int, iconWidth);
	QFETCH(int, iconHeight);

	CurrentColorWidget widget{iconWidth, iconHeight};

	auto const path = iconWidth == iconHeight ? std::format("images/CurrentColor-{}px.png", iconWidth)
	                                          : std::format("images/CurrentColor-{}px-{}px.png", iconWidth, iconHeight);
	Stamper stamper{
	    &widget,
	    {
	        .path = path,
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
