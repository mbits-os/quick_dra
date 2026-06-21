// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QEnterEvent>
#include <QPainter>
#include <QSignalSpy>
#include <QTest>
#include <app/controls/Glyph.hpp>
#include <app/controls/PanelButtonGroup.hpp>
#include <app/gui/CurrentColor.hpp>
#include <app/utils/LaidOut.hpp>
#include <app/utils/utils.hpp>
#include <array>
#include <chrono>
#include <print>
#include <vector>
#include "ControlsTest.hpp"
#include "palette_override.hpp"
#include "stamper.hpp"

using namespace quick_dra::gui;
using namespace quick_dra;
using namespace std::literals;

#ifndef OS_NAME
#error Must define OS_NAME
#endif

struct Cursor {
	PanelButtonGroup* wgt{};
	Qt::MouseButtons buttons{};

	void enter(QPointF const& local) {
		QEnterEvent event{local, local, wgt->mapToGlobal(local)};
		notify(&event);
	}

	void leave() {
		QEvent event{QEvent::Leave};
		notify(&event);
	}

	void move(QPointF const& local, Qt::MouseButton button = Qt::NoButton) {
		mouseEvent(QEvent::MouseMove, local, button);
	}

	void buttonPress(QPointF const& local, Qt::MouseButton button) {
		mouseEvent(QEvent::MouseButtonPress, local, button);
	}

	void buttonRelease(QPointF const& local, Qt::MouseButton button) {
		mouseEvent(QEvent::MouseButtonRelease, local, button);
		buttons &= ~button;
	}

private:
	void mouseEvent(QEvent::Type type, QPointF const& local, Qt::MouseButton button = Qt::NoButton) {
		buttons |= button;
		QMouseEvent event{type, local, wgt->mapToGlobal(local), button, buttons, Qt::NoModifier};
		notify(&event);
	}

	void notify(QEvent* event) {
		switch (event->type()) {
			case QEvent::Enter:
				return wgt->enterEvent(static_cast<QEnterEvent*>(event));
			case QEvent::Leave:
				return wgt->leaveEvent(event);
			case QEvent::MouseMove:
				return wgt->mouseMoveEvent(static_cast<QMouseEvent*>(event));
			case QEvent::MouseButtonPress:
				return wgt->mousePressEvent(static_cast<QMouseEvent*>(event));
			case QEvent::MouseButtonRelease:
				return wgt->mouseReleaseEvent(static_cast<QMouseEvent*>(event));
			default:
				break;
		}
	}
};

static constexpr auto iconCreators = std::array{
    gui::arrowRightSVGIcon, gui::arrowLeftSVGIcon, gui::ellipsisSVGIcon,
    gui::checkSVGIcon,      gui::resetSVGIcon,     gui::warningSVGIcon,
};

struct Buttons {
	std::vector<PanelButton*> buttons{};
	Panel* second{};

	explicit Buttons(PanelButtonGroup& widget) {
		buttons.reserve(8);
		buttons.push_back(widget.createWidget<Panel>([](Panel& panel) {
			panel.setInfo("Label #1 (original)", "*tag*: info, *iteration*: #1", "Value A", arrowRightSVGIcon());
			panel.setInfo("Label #1 (changed)", "*tag*: info, *iteration*: #2", "Value B", arrowRightSVGIcon());
		}));
		buttons.push_back(widget.createWidget<Panel>(
		    [](Panel& panel) { panel.setInfo("Label #2", {}, "123,50 zł", arrowRightSVGIcon()); }));
		buttons.push_back(widget.createWidget<Panel>([](Panel& panel) {
			panel.setInfo(
			    "Label #3",
			    "*date*: 2002-01-15, *tags*: alpha beta gamma delta long-words long list many words wrap around", {},
			    arrowRightSVGIcon());
		}));
		buttons.push_back(widget.createWidget<Panel>([](Panel& panel) {
			panel.setInfo(
			    "Label #3 (but hidden)",
			    "*date*: 2002-01-15, *tags*: alpha beta gamma delta long-words long list many words wrap around", {},
			    arrowRightSVGIcon());
			panel.setHidden(true);
		}));
		buttons.push_back(
		    widget.createWidget<Panel>([](Panel& panel) { panel.setInfo("Open dialog", {}, {}, ellipsisSVGIcon()); }));

		auto const layout = new QHBoxLayout{};
		layout->setContentsMargins(5, 5, 5, 5);
		layout->setSpacing(5);
		for (auto creator : iconCreators) {
			auto const glyph = new Glyph{&widget};
			glyph->setIcon(creator());
			glyph->setIconSize(20, 20);
			layout->addWidget(glyph);
		}

		buttons.push_back(widget.addLayout(layout));
		buttons.push_back(widget.addButton("Normal label", false));
		buttons.push_back(widget.addButton("Bold label", true));
	}
};

static PaletteOverride const themes[] = {
    {.window = Qt::lightGray, .windowText = Qt::black},
    {.window = QColor{18, 18, 18, 255}, .windowText = QColor{224, 224, 224, 255}},
};

using ButtonOp = void (*)(Buttons&);
static constexpr ButtonOp actions[] = {
    [](Buttons& btns) {
	    for (auto button : btns.buttons) {
		    button->setClickable(true);
		    button->setEnabled(true);
		    button->setHovered(false);
		    button->setActive(false);
	    }
	    auto const widget = static_cast<Panel*>(btns.buttons[1]->widget());
	    widget->value()->setText("123,50 zł");
    },
    [](Buttons& btns) {
	    btns.buttons[0]->setClickable(false);
	    btns.buttons[0]->setHovered(true);
	    btns.buttons[1]->setHovered(true);
	    btns.buttons[2]->setEnabled(false);
	    btns.buttons[2]->setHovered(true);
    },
    [](Buttons& btns) {
	    btns.buttons[1]->setHovered(false);
	    btns.buttons[1]->setActive(true);
    },
    [](Buttons& btns) {
	    btns.buttons[1]->setActive(false);
	    btns.buttons[1]->setHovered(true);
	    auto const widget = static_cast<Panel*>(btns.buttons[1]->widget());
	    widget->value()->setText("Clicked!");
    },
};

void ControlsTest::PanelButtonGroup_layout() {
	PanelButtonGroup widget{};
	widget.resize(450, 1);
	Buttons btns{widget};
	widget.show();
	QVERIFY(QTest::qWaitForWindowExposed(&widget));

	Stamper stamper{&widget,
	                {
	                    .path = "images/PanelButtonGroup.layout_" OS_NAME ".png"sv,
	                    .columns = std::size(actions) + 1,
	                    .rows = std::size(themes),
	                }};

	for (auto const& pos : stamper) {
		if (safe_size_t(pos.column) >= std::size(actions)) continue;
		themes[safe_size_t(pos.row)].install(&widget);
		actions[safe_size_t(pos.column)](btns);
		stamper.grab(pos);
	}

	widget.clearAll();
	QTest::qWait(0);

	int column = std::size(actions);
	int row = 0;
	for (auto const& theme : themes) {
		theme.install(&widget);
		stamper.grab(row, column);
		++row;
	}

	auto stencil = stamper.loadStencil();
	COMPARE_IMAGES(stencil, stamper);
}

void ControlsTest::PanelButtonGroup_contents() {
	PanelButtonGroup widget{};
	widget.resize(450, 1);
	Buttons btns{widget};
	int layoutId = 5;
	int pos = -1;
	for (auto const button : btns.buttons) {
		++pos;
		if (pos == layoutId) {
			QVERIFY2(button->layout(), std::format("index = {}", pos).c_str());
		} else {
			QVERIFY2(button->widget(), std::format("index = {}", pos).c_str());
		}
	}
}

void ControlsTest::PanelButtonGroup_mouseMove() {
	PanelButtonGroup widget{};
	widget.resize(450, 1);
	Cursor click{.wgt = &widget};
	Buttons btns{widget};
	btns.buttons[2]->setEnabled(false);
	widget.show();
	QVERIFY(QTest::qWaitForWindowExposed(&widget));

	QCOMPARE_EQ(widget.height(), 648);

	struct pti {
		int y{0};
		size_t index{0};
	};
	static constexpr auto y_positions = std::array{
	    pti{.y = 0, .index = 8},   pti{.y = 50, .index = 0},  pti{.y = 100, .index = 1},
	    pti{.y = 300, .index = 2}, pti{.y = 390, .index = 4}, pti{.y = 500, .index = 5},
	    pti{.y = 540, .index = 6}, pti{.y = 590, .index = 7}, pti{.y = 653, .index = 8},
	};

	int x = widget.width() / 2;

	click.enter(QPoint{0, y_positions[0].y}.toPointF());

	for (auto const [y, expected] : y_positions) {
		auto const pt = QPoint{x, y}.toPointF();

		click.move(pt);

		size_t hovered = btns.buttons.size();
		size_t count = 0;

		size_t pos = 0;
		for (auto const button : btns.buttons) {
			if (button->isHovered()) {
				hovered = pos;
				++count;
			}
			++pos;
		}

		QCOMPARE_EQ(hovered, expected);

		if (count == 0) {
			QCOMPARE_GE(expected, btns.buttons.size());
		} else if (count == 1) {
			QCOMPARE_LT(expected, btns.buttons.size());
		} else {
			QCOMPARE_EQ(count, 1);
		}
	}
	click.leave();
}

void ControlsTest::PanelButtonGroup_mouseClick() {
	PanelButtonGroup widget{};
	widget.resize(450, 1);
	Cursor click{.wgt = &widget};
	Buttons btns{widget};
	btns.buttons[2]->setClickable(true);
	widget.show();
	QVERIFY(QTest::qWaitForWindowExposed(&widget));

	QSignalSpy spy{btns.buttons[2], &PanelButton::clicked};

	QVERIFY(spy.isValid());

	int x = widget.width() / 2;
	int y = 300;

	auto const pt = QPoint{x, y}.toPointF();
	click.enter({0, 0});
	click.buttonPress(pt, Qt::LeftButton);
	click.buttonRelease(pt, Qt::LeftButton);
	click.leave();

	QTest::qWait(0);
	QCOMPARE_EQ(spy.size(), 1);
}

void ControlsTest::PanelButtonGroup_mouseClickWrongPlace() {
	PanelButtonGroup widget{};
	widget.resize(450, 1);
	Cursor click{.wgt = &widget};
	Buttons btns{widget};
	btns.buttons[2]->setClickable(true);
	widget.show();
	QVERIFY(QTest::qWaitForWindowExposed(&widget));

	QSignalSpy spy{btns.buttons[2], &PanelButton::clicked};

	QVERIFY(spy.isValid());

	int x = widget.width() / 2;

	auto const pt1 = QPoint{x, 300}.toPointF();
	auto const pt2 = QPoint{x, 0}.toPointF();
	click.enter({0, 0});
	click.buttonPress(pt2, Qt::LeftButton);
	click.move(pt1);
	click.buttonRelease(pt1, Qt::LeftButton);
	click.leave();

	QTest::qWait(0);
	QCOMPARE_EQ(spy.size(), 0);
}

void ControlsTest::PanelButtonGroup_mouseClickLeftRight() {
	PanelButtonGroup widget{};
	widget.resize(450, 1);
	Cursor click{.wgt = &widget};
	Buttons btns{widget};
	btns.buttons[2]->setClickable(true);
	widget.show();
	QVERIFY(QTest::qWaitForWindowExposed(&widget));

	QSignalSpy spy{btns.buttons[2], &PanelButton::clicked};

	QVERIFY(spy.isValid());

	int x = widget.width() / 2;
	int y = 300;

	auto const pt = QPoint{x, y}.toPointF();
	click.enter({0, 0});
	click.buttonPress(pt, Qt::LeftButton);
	click.buttonPress(pt, Qt::RightButton);
	click.buttonRelease(pt, Qt::RightButton);

	QTest::qWait(0);
	QCOMPARE_EQ(spy.size(), 0);

	click.buttonRelease(pt, Qt::LeftButton);
	click.leave();

	QTest::qWait(0);
	QCOMPARE_EQ(spy.size(), 1);
}

void ControlsTest::PanelButtonGroup_mouseClickMoveAway() {
	PanelButtonGroup widget{};
	widget.resize(450, 1);
	Cursor click{.wgt = &widget};
	Buttons btns{widget};
	btns.buttons[2]->setClickable(true);
	widget.show();
	QVERIFY(QTest::qWaitForWindowExposed(&widget));

	QSignalSpy spy{btns.buttons[2], &PanelButton::clicked};

	QVERIFY(spy.isValid());

	int x = widget.width() / 2;

	auto const pt1 = QPoint{x, 300}.toPointF();
	auto const pt2 = QPoint{x, 390}.toPointF();
	click.enter({0, 0});
	click.buttonPress(pt1, Qt::LeftButton);
	click.move(pt2);
	click.buttonRelease(pt2, Qt::LeftButton);
	click.leave();

	QTest::qWait(0);
	QCOMPARE_EQ(spy.size(), 0);
}

void ControlsTest::PanelButtonGroup_mouseClickMoveAwayAndReturn() {
	PanelButtonGroup widget{};
	widget.resize(450, 1);
	Cursor click{.wgt = &widget};
	Buttons btns{widget};
	btns.buttons[2]->setClickable(true);
	widget.show();
	QVERIFY(QTest::qWaitForWindowExposed(&widget));

	QSignalSpy spy{btns.buttons[2], &PanelButton::clicked};

	QVERIFY(spy.isValid());

	int x = widget.width() / 2;

	auto const pt1 = QPoint{x, 300}.toPointF();
	auto const pt2 = QPoint{x, 390}.toPointF();
	click.enter({0, 0});
	click.buttonPress(pt1, Qt::LeftButton);
	click.move(pt2);
	click.move(pt1);
	click.buttonRelease(pt1, Qt::LeftButton);
	click.leave();

	QTest::qWait(0);
	QCOMPARE_EQ(spy.size(), 1);
}

void ControlsTest::PanelButtonGroup_mouseClickMoveOutside() {
	PanelButtonGroup widget{};
	widget.resize(450, 1);
	Cursor click{.wgt = &widget};
	Buttons btns{widget};
	btns.buttons[2]->setClickable(true);
	widget.show();
	QVERIFY(QTest::qWaitForWindowExposed(&widget));

	QSignalSpy spy{btns.buttons[2], &PanelButton::clicked};

	QVERIFY(spy.isValid());

	int x = widget.width() / 2;

	auto const pt1 = QPoint{x, 300}.toPointF();
	auto const pt2 = QPoint{x + widget.width(), 300}.toPointF();
	click.enter({0, 0});
	click.buttonPress(pt1, Qt::LeftButton);
	click.move(pt2);
	click.buttonRelease(pt2, Qt::LeftButton);
	click.leave();

	QTest::qWait(0);
	QCOMPARE_EQ(spy.size(), 0);
}
