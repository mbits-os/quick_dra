// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QMainWindow>

#include <QDebug>
#include <QTest>
#include <QVBoxLayout>
#include <app/gui/PageHeader.hpp>
#include <app/utils/utils.hpp>
#include "GuiTest.hpp"

// #include "PageHeader.test.moc"

using namespace quick_dra::gui;
using namespace std::literals;

void GuiTest::PageHeader_movingShadow() {
	QMainWindow window{};
	auto parent = new QWidget{&window};
	parent->setObjectName("centralWidget");
	window.setCentralWidget(parent);

	auto layout = new QVBoxLayout{parent};
	layout->setObjectName("layout");

	auto shadow = new HeaderShadow{parent};
	shadow->setObjectName("shadow");
	shadow->setShadowHeight(12);
	shadow->setShadowForce(.25f);

	auto pageHeader = new PageHeader{parent};
	pageHeader->setObjectName("header");
	pageHeader->setSizePolicy(TakeWidth);
	pageHeader->setTopMost(true);

	layout->addWidget(pageHeader);
	layout->addWidget(new QWidget{parent}, 1);

	QObject::connect(pageHeader, &PageHeader::moved, shadow, &HeaderShadow::targetMoved);

	// window.resize(QGuiApplication::primaryScreen()->availableSize());
	window.show();
	QVERIFY(QTest::qWaitForWindowExposed(&window));

	auto const headerGeo = pageHeader->geometry();
	auto const shadowGeo = shadow->geometry();
	QCOMPARE_EQ(shadowGeo.topLeft(), headerGeo.bottomLeft());
	QCOMPARE_EQ(shadowGeo.topRight(), headerGeo.bottomRight());
	QCOMPARE_EQ(shadowGeo.height(), shadow->shadowHeight());
	QCOMPARE_EQ(shadow->shadowForce(), 0.25);
	QCOMPARE_EQ(shadow->minimumSizeHint(), QSize(1, 12));
}

void GuiTest::PageHeader_centeringTitle() {
	QMainWindow window{};
	auto pageHeader = new PageHeader{&window};
	PARENT_CONTEXT(pageHeader);
	ENSURE_CHILD(HeaderToolbar, toolBar);
	ENSURE_CHILD(HeaderSpacer, spacer);

	pageHeader->setObjectName("centralWidget");
	pageHeader->setTopMost(true);
	window.setCentralWidget(pageHeader);

	window.show();
	QVERIFY(QTest::qWaitForWindowExposed(&window));

	QCOMPARE_EQ(spacer->width(), toolBar->width());
	QVERIFY(pageHeader->topMost());
	QVERIFY(!pageHeader->formDirty());
	QVERIFY(pageHeader->formValid());

	auto const origSpace = toolBar->width();
	pageHeader->setTopMost(false);
	QTest::qWait(100ms);
	QCOMPARE_NE(toolBar->width(), origSpace);
	QCOMPARE_EQ(spacer->width(), toolBar->width());

	auto const subPageSpace = toolBar->width();
	pageHeader->setFormDirty(true);
	QTest::qWait(100ms);
	QCOMPARE_NE(toolBar->width(), origSpace);
	QCOMPARE_NE(toolBar->width(), subPageSpace);
	QCOMPARE_EQ(spacer->width(), toolBar->width());
}
