// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QTest>
#include <QVBoxLayout>
#include <app/controls/PageScrollArea.hpp>
#include <app/utils/LaidOut.hpp>
#include <app/utils/utils.hpp>
#include <chrono>
#include "ControlsTest.hpp"

using namespace quick_dra::gui;
using namespace quick_dra;
using namespace std::literals;

void ControlsTest::PageScrollArea_simpleResize() {
	QWidget page{};
	auto const [scrollArea, parent] = PageScrollArea::setupPage(&page);
	parent->setMaximumHeight(200);

	page.resize(200, 600);
	page.show();
	QVERIFY(QTest::qWaitForWindowExposed(&page));

	QCOMPARE_EQ(parent->rect(), (QRect{0, 0, 200, 200}));
}

class LongTextWidget : public QWidget {
	Q_OBJECT

public:
	int area = 100'000;
	LongTextWidget(QWidget* parent) : QWidget{parent} { setSizePolicy(TakeAll / HeightForWidth); };
	int heightForWidth(int width) const override { return area / std::max(1, width); }
};

#include "PageScrollArea.test.moc"

void ControlsTest::PageScrollArea_longText() {
	QWidget page{};
	auto const [scrollArea, parent] = PageScrollArea::setupPage(&page);
	parent->setMaximumHeight(200);
	parent->setSizePolicy(TakeWidth / HeightForWidth);

	QVBoxLayout* layout{};
	LongTextWidget* longText{};
	LaidOut{parent}.createLayout(layout, "layout", parent,
	                             [](QLayout& layout) { layout.setContentsMargins(5, 5, 5, 5); });
	LaidOut{parent, layout}.createWidget(longText, "longText");

	page.resize(200, 600);
	page.show();
	QVERIFY(QTest::qWaitForWindowExposed(&page));

	QCOMPARE_EQ(longText->heightForWidth(190), 526);
	QCOMPARE_EQ(parent->heightForWidth(200), 536);
	QCOMPARE_EQ(longText->rect(), (QRect{0, 0, 190, 190}));
}
