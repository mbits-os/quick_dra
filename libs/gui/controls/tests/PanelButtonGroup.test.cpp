// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QEnterEvent>
#include <QPainter>
#include <QSignalSpy>
#include <QTest>
#include <app/controls/DevicePixelScale.hpp>
#include <app/controls/Glyph.hpp>
#include <app/controls/Panel.hpp>
#include <app/controls/PanelButtonGroup.hpp>
#include <app/controls/PanelButtonStyle.hpp>
#include <app/gui/CurrentColor.hpp>
#include <app/utils/LaidOut.hpp>
#include <app/utils/utils.hpp>
#include <array>
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

void setBg(auto* widget, QColor baseColor, int alpha = 20) {
	auto pal = widget->palette();
	baseColor.setAlpha(alpha);
	pal.setColor(QPalette::Window, baseColor);
	widget->setPalette(pal);
	widget->setAutoFillBackground(true);
}

struct Buttons {
	std::vector<PanelButton*> buttons{};

	explicit Buttons(PanelButtonGroup& widget) {
		auto const scale = DevicePixelScale{widget.logicalDpiX()};
		static constexpr auto contentsWidth = 215_px;
		static constexpr auto width = contentsWidth + PanelButtonStyle::Margin * 2;
		widget.resize(scale.toDevice(width), 1);
		buttons.reserve(8);
		buttons.push_back(widget.createPanel({.label = "Label #1 (original)",
		                                      .details = "*tag*: info, *iteration*: #1",
		                                      .value = "Value A",
		                                      .rightIcon = arrowRightSVGIcon()}));
		static_cast<Panel*>(buttons.back()->widget())
		    ->setInfo({.label = "Label #1 (changed)",
		               .details = "*tag*: info, *iteration*: #2",
		               .value = "Value B",
		               .rightIcon = arrowRightSVGIcon()});
		buttons.push_back(
		    widget.createPanel({.label = "Label #2", .value = "123,50 zł", .rightIcon = arrowRightSVGIcon()}));
		buttons.push_back(
		    widget.createPanel({.label = "Label #3",
		                        .details = "*date*: 2002-01-15, *tags*: alpha beta gamma delta long-words long list "
		                                   "many words wrap around",
		                        .rightIcon = arrowRightSVGIcon()}));
		buttons.push_back(
		    widget.createPanel({.label = "Label #3 (but hidden)",
		                        .details = "*date*: 2002-01-15, *tags*: alpha beta gamma delta long-words long list "
		                                   "many words wrap around",
		                        .rightIcon = arrowRightSVGIcon()}));
		buttons.back()->widget()->setHidden(true);
		buttons.push_back(widget.createPanel({.label = "Open dialog", .rightIcon = ellipsisSVGIcon()}));

		auto const layout = new QHBoxLayout{};
		auto const spacing = scale.toDevice(5_px);
		auto const iconSize = scale.toDevice(16_px);
		layout->setContentsMargins(spacing, spacing, spacing, spacing);
		layout->setSpacing(spacing);
		for (auto creator : iconCreators) {
			auto const glyph = new Glyph{&widget};
			glyph->setIcon(creator());
			glyph->setIconSize(iconSize, iconSize);
			layout->addWidget(glyph);
			glyph->setSizePolicy(TakeWidth);
		}

		buttons.push_back(widget.addLayout(layout));
		buttons.push_back(widget.addButton("Normal label", false));
		buttons.push_back(widget.addButton("Bold label", true));
	}

	void resize(PanelButtonGroup& widget, qreal scale = 1) {
		int height = 0;
		auto const widgetLayout = widget.layout();
		auto const margins = widgetLayout->contentsMargins();
		auto const spacing = widgetLayout->spacing();
		auto const width = widget.width() - (margins.left() + margins.right());

		auto const count = widgetLayout->count();
		for (auto index = 0; index < count; ++index) {
			auto const item = widgetLayout->itemAt(index);
			auto const child = item->widget();
			if (child && child->isHidden()) continue;
			if (height) height += spacing;
			height += item->hasHeightForWidth() ? item->heightForWidth(width) : item->sizeHint().height();
		}
		height += margins.top() + margins.bottom();

		height = qRound(height * scale);
		widget.resize(widget.width(), height);
	}
};

static PaletteOverride const themes[] = {
    {.window = Qt::white, .windowText = Qt::black},
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
	    widget->setValue("123,50 zł");
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
	    widget->setValue("Clicked!");
    },
};

void ControlsTest::DevicePixelScale() {
	QCOMPARE_EQ(12_px, 9_pt);
	QCOMPARE_EQ(1_in, 96_px);
	QCOMPARE_EQ(1_in, 72_pt);
	QCOMPARE_EQ(std::format("{}", 2.4_in), "2.4in"sv);
	QCOMPARE_EQ(std::format("{}", 15_px), "15px"sv);
	QCOMPARE_EQ(std::format("{}", 32_pt), "32pt"sv);
	QCOMPARE_EQ(std::format("{}", Length<std::ratio<15, 17>>(45)), "45[15/17]in"sv);
}

void ControlsTest::PanelButtonGroup_layout() {
	PanelButtonGroup widget{};
	Buttons btns{widget};
	btns.buttons.front()->widget()->setFont(QFont{"DejaVu Sans Mono", qApp->font().pointSize()});
	widget.show();
	QVERIFY(QTest::qWaitForWindowExposed(&widget));
	btns.resize(widget);

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

QPointF midpoint(QWidget* widget) {
	auto const geo = widget->geometry().toRectF();
	return (geo.topLeft() + geo.bottomRight()) / 2;
}

void ControlsTest::PanelButtonGroup_mouseMove() {
	PanelButtonGroup widget{};
	Cursor click{.wgt = &widget};
	Buttons btns{widget};
	btns.buttons[2]->setEnabled(false);
	widget.show();
	QVERIFY(QTest::qWaitForWindowExposed(&widget));

	struct pti {
		int y{0};
		size_t index{0};
	};

	auto const make_pti = [btns](size_t index) {
		auto const button = btns.buttons[index];
		auto const widget = button->widget();
		auto const layout = button->layout();
		auto geo = widget ? widget->geometry() : layout->geometry();
		return pti{(geo.top() + geo.bottom()) / 2, index};
	};

	auto const y_positions = std::array{
	    pti{.y = 0, .index = 8},
	    make_pti(0),
	    make_pti(1),
	    make_pti(2),
	    make_pti(4),
	    make_pti(5),
	    make_pti(6),
	    make_pti(7),
	    pti{.y = std::numeric_limits<int>::max(), .index = 8},
	};

	int x = widget.width() / 2;

	click.enter({0, 0});

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
	Cursor click{.wgt = &widget};
	Buttons btns{widget};
	auto third = btns.buttons[2];
	widget.show();
	QVERIFY(QTest::qWaitForWindowExposed(&widget));

	QSignalSpy spy{third, &PanelButton::clicked};
	QVERIFY(spy.isValid());

	auto const pt = midpoint(third->widget());
	click.enter({0, 0});
	click.buttonPress(pt, Qt::LeftButton);
	click.buttonRelease(pt, Qt::LeftButton);
	click.leave();

	QTest::qWait(0);
	QCOMPARE_EQ(spy.size(), 1);
}

void ControlsTest::PanelButtonGroup_mouseClickWrongPlace() {
	PanelButtonGroup widget{};
	Cursor click{.wgt = &widget};
	Buttons btns{widget};
	auto third = btns.buttons[2];
	widget.show();
	QVERIFY(QTest::qWaitForWindowExposed(&widget));

	QSignalSpy spy{third, &PanelButton::clicked};
	QVERIFY(spy.isValid());

	auto const pt1 = midpoint(third->widget());
	auto const pt2 = QPointF{pt1.x(), 0};
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
	Cursor click{.wgt = &widget};
	Buttons btns{widget};
	auto third = btns.buttons[2];
	widget.show();
	QVERIFY(QTest::qWaitForWindowExposed(&widget));

	QSignalSpy spy{third, &PanelButton::clicked};
	QVERIFY(spy.isValid());

	auto const pt = midpoint(third->widget());
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
	Cursor click{.wgt = &widget};
	Buttons btns{widget};
	auto third = btns.buttons[2];
	widget.show();
	QVERIFY(QTest::qWaitForWindowExposed(&widget));

	QSignalSpy spy{third, &PanelButton::clicked};
	QVERIFY(spy.isValid());

	auto const pt1 = midpoint(third->widget());
	auto const pt2 = midpoint(btns.buttons[3]->widget());
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
	Cursor click{.wgt = &widget};
	Buttons btns{widget};
	auto third = btns.buttons[2];
	widget.show();
	QVERIFY(QTest::qWaitForWindowExposed(&widget));

	QSignalSpy spy{third, &PanelButton::clicked};
	QVERIFY(spy.isValid());

	auto const pt1 = midpoint(third->widget());
	auto const pt2 = midpoint(btns.buttons[3]->widget());
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
	Cursor click{.wgt = &widget};
	Buttons btns{widget};
	auto third = btns.buttons[2];
	widget.show();
	QVERIFY(QTest::qWaitForWindowExposed(&widget));

	QSignalSpy spy{btns.buttons[2], &PanelButton::clicked};

	QVERIFY(spy.isValid());

	auto const pt1 = midpoint(third->widget());
	auto const pt2 = QPointF{pt1.x() + widget.width(), pt1.y()};

	click.enter({0, 0});
	click.buttonPress(pt1, Qt::LeftButton);
	click.move(pt2);
	click.buttonRelease(pt2, Qt::LeftButton);
	click.leave();

	QTest::qWait(0);
	QCOMPARE_EQ(spy.size(), 0);
}
