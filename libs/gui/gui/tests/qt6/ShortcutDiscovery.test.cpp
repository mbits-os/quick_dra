// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QDebug>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTest>
#include <QWheelEvent>
#include <Qt>
#include <app/gui/ShortcutDiscovery.hpp>
#include <app/utils/utils.hpp>
#include "GuiTest.hpp"
#include "ui_helpers.hpp"

using namespace quick_dra::gui;
using namespace std::literals;

namespace {
	static constexpr auto modMap = std::array{
	    std::pair{Qt::Key_Shift, Qt::ShiftModifier},
	    std::pair{Qt::Key_Control, Qt::ControlModifier},
	    std::pair{Qt::Key_Meta, Qt::MetaModifier},
	    std::pair{Qt::Key_Alt, Qt::AltModifier},
	};

	constexpr Qt::KeyboardModifier fromKey(Qt::Key key) noexcept {
		for (auto const& [modKey, modifier] : modMap) {
			if (modKey == key) return modifier;
		}

		return Qt::NoModifier;
	}
}  // namespace

struct HUD {
	QObject* target{};
	Qt::KeyboardModifiers modifiers{};

	// HUD(QObject* target) : target{target} {}

	void press(Qt::Key key, quint16 count = 1) {
		modifiers |= fromKey(key);
		keyboardEvent(QEvent::KeyPress, key, count);
	}

	void release(Qt::Key key, quint16 count = 1) {
		modifiers &= ~fromKey(key);
		keyboardEvent(QEvent::KeyRelease, key, count);
	}

	void mouseEvent(QEvent::Type type) {
		QMouseEvent ev{type, QPointF{}, QPointF{}, Qt::LeftButton, Qt::LeftButton | Qt::NoButton, modifiers};
		notify(&ev);
	}

	void wheelEvent() {
		QWheelEvent ev{
		    QPointF{}, QPointF{}, QPoint{}, QPoint{}, Qt::LeftButton | Qt::NoButton, modifiers, Qt::ScrollUpdate, false,
		};
		notify(&ev);
	}

private:
	void keyboardEvent(QEvent::Type type, Qt::Key key, quint16 count) {
		QKeyEvent event{type, key, modifiers, QString{}, count > 1, count};
		notify(&event);
	}
	void notify(QEvent* event) { qApp->notify(target, event); }
};

QVariant modsArg(Qt::KeyboardModifiers mods) { return QVariant{mods}; }
QList<QVariant> modsChanged(Qt::KeyboardModifiers mods) { return {modsArg(mods)}; }

void GuiTest::ShortcutDiscovery_shiftCtrlAltD() {
	ShortcutDiscovery discovery{100ms};
	QSignalSpy spy{&discovery, &ShortcutDiscovery::modifiersChanged};
	QWidget widget{};
	HUD hud{&widget};
	widget.show();
	QVERIFY(QTest::qWaitForWindowExposed(&widget));

	QVERIFY(!discovery.isActive());

	hud.press(Qt::Key_Shift);
	hud.press(Qt::Key_Alt);

	QVERIFY(!discovery.isActive());

	spy.wait(200ms);

	QVERIFY(discovery.isActive());

	hud.press(Qt::Key_Control);
	hud.press(Qt::Key_D);

	spy.wait(100ms);

	QVERIFY(!discovery.isActive());

	while (spy.length() < 3) {
		spy.wait(100ms);
	}
	QVERIFY(!discovery.isActive());

	QCOMPARE_EQ(spy, (QList{
	                     modsChanged(Qt::ShiftModifier | Qt::AltModifier),
	                     modsChanged(Qt::ShiftModifier | Qt::AltModifier | Qt::ControlModifier),
	                     modsChanged(Qt::NoModifier),
	                 }));
}

void GuiTest::ShortcutDiscovery_removeModifiers() {
	ShortcutDiscovery discovery{100ms};
	QSignalSpy spy{&discovery, &ShortcutDiscovery::modifiersChanged};
	QWidget widget{};
	HUD hud{&widget};
	widget.show();
	QVERIFY(QTest::qWaitForWindowExposed(&widget));

	QVERIFY(!discovery.isActive());

	hud.press(Qt::Key_Shift);
	hud.press(Qt::Key_Alt);
	hud.press(Qt::Key_Control);
	hud.press(Qt::Key_Meta);

	spy.wait(200ms);
	QVERIFY(discovery.isActive());

	hud.release(Qt::Key_Shift);
	hud.release(Qt::Key_Alt);
	hud.release(Qt::Key_Control);
	hud.release(Qt::Key_Meta);

	while (spy.length() < 5) {
		spy.wait(100ms);
	}
	QVERIFY(!discovery.isActive());

	QCOMPARE_EQ(spy, (QList{
	                     modsChanged(Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier),
	                     modsChanged(Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier),
	                     modsChanged(Qt::ControlModifier | Qt::MetaModifier),
	                     modsChanged(Qt::MetaModifier),
	                     modsChanged(Qt::NoModifier),
	                 }));
}

void GuiTest::ShortcutDiscovery_removeKey() {
	ShortcutDiscovery discovery{100ms};
	QSignalSpy spy{&discovery, &ShortcutDiscovery::modifiersChanged};
	QWidget widget{};
	HUD hud{&widget};
	widget.show();
	QVERIFY(QTest::qWaitForWindowExposed(&widget));

	QVERIFY(!discovery.isActive());

	hud.press(Qt::Key_A);
	hud.press(Qt::Key_Control);

	spy.wait(200ms);
	QVERIFY(discovery.isActive());

	hud.release(Qt::Key_A);

	while (spy.length() < 2) {
		spy.wait(100ms);
	}
	QVERIFY(!discovery.isActive());

	QCOMPARE_EQ(spy, (QList{
	                     modsChanged(Qt::ControlModifier),
	                     modsChanged(Qt::NoModifier),
	                 }));
}

void GuiTest::ShortcutDiscovery_mouseButtons_data() {
	QTest::addColumn<QEvent::Type>("type");

	QTest::newRow("Press") << QEvent::MouseButtonPress;
	QTest::newRow("Release") << QEvent::MouseButtonRelease;
	QTest::newRow("DblClick") << QEvent::MouseButtonDblClick;
	// QTest::newRow("Wheel") << QEvent::Wheel;
}

void GuiTest::ShortcutDiscovery_mouseButtons() {
	QFETCH(QEvent::Type, type);

	ShortcutDiscovery discovery{100ms};
	QSignalSpy spy{&discovery, &ShortcutDiscovery::modifiersChanged};
	QWidget widget{};
	HUD hud{&widget};
	widget.show();
	QVERIFY(QTest::qWaitForWindowExposed(&widget));

	QVERIFY(!discovery.isActive());

	hud.press(Qt::Key_Control);

	QVERIFY(!discovery.isActive());

	spy.wait(200ms);

	QVERIFY(discovery.isActive());
	hud.mouseEvent(type);

	while (spy.length() < 2) {
		spy.wait(100ms);
	}
	QVERIFY(!discovery.isActive());

	QCOMPARE_EQ(spy, (QList{modsChanged(Qt::ControlModifier), modsChanged(Qt::NoModifier)}));
}

void GuiTest::ShortcutDiscovery_mouseWheel() {
	ShortcutDiscovery discovery{100ms};
	QSignalSpy spy{&discovery, &ShortcutDiscovery::modifiersChanged};
	QWidget widget{};
	HUD hud{&widget};
	widget.show();
	QVERIFY(QTest::qWaitForWindowExposed(&widget));

	QVERIFY(!discovery.isActive());

	hud.press(Qt::Key_Control);

	QVERIFY(!discovery.isActive());

	spy.wait(200ms);

	QVERIFY(discovery.isActive());
	hud.wheelEvent();

	while (spy.length() < 2) {
		spy.wait(100ms);
	}
	QVERIFY(!discovery.isActive());

	QCOMPARE_EQ(spy, (QList{modsChanged(Qt::ControlModifier), modsChanged(Qt::NoModifier)}));
}