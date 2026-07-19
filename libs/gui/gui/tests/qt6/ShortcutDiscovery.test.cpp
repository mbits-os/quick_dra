// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QDebug>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTest>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <Qt>
#include <app/gui/ShortcutDiscovery.hpp>
#include <app/gui/ToolTipLabel.hpp>
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

	QString operator""_L1(char const* ptr, size_t len) { return QString::fromLatin1(ptr, static_cast<qsizetype>(len)); }
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

class ShortcutWidget : public QWidget {
public:
	ShortcutWidget(QSize const& hint, QList<QKeySequence> const& keys, QWidget* parent = nullptr)
	    : QWidget{parent}, keys{keys}, hint_{hint} {
		setSizePolicy(TakeWidth);
	}

	QSize sizeHint() const override { return hint_; }
	QSize minimumSizeHint() const override { return hint_; }

	QList<QKeySequence> keys;
	QSize hint_;
};

namespace quick_dra::gui {
	template <>
	struct HolderSupport<ShortcutWidget> : HolderSupport<QWidget> {
		static QList<QKeySequence> keys(ShortcutWidget const* holder) { return holder->keys; }
	};
}  // namespace quick_dra::gui

enum class EnabledVisible { None = 0, Disabled = 1, Invisible = 2 };
Q_DECLARE_FLAGS(EV, EnabledVisible)
Q_DECLARE_OPERATORS_FOR_FLAGS(EV)

static std::tuple<int, QList<QKeySequence>, EV> const controls[] = {
    {36, {Qt::CTRL | Qt::SHIFT | Qt::Key_E}, EnabledVisible::None},
    {36, {Qt::CTRL | Qt::ALT | Qt::Key_Z, Qt::CTRL | Qt::Key_Z}, {}},
    {24, {Qt::CTRL | Qt::Key_K}, EnabledVisible::Disabled},
    {48, {Qt::CTRL | Qt::Key_C}, EnabledVisible::Invisible},
    {36, {Qt::CTRL | Qt::Key_D}, EnabledVisible::Disabled | EnabledVisible::Invisible},
    {24, {Qt::CTRL | Qt::Key_V}, EnabledVisible::None},
};

auto widgetMaker(QWidget* parent, QLayout* layout) {
	return [parent, layout](std::tuple<int, QList<QKeySequence>, EV> const& control) {
		auto const& [height, keys, ev] = control;
		auto* result = new ShortcutWidget{QSize{1, height}, keys, parent};
		if (ev & EnabledVisible::Disabled) {
			result->setEnabled(false);
		}
		if (ev & EnabledVisible::Invisible) {
			result->setVisible(false);
		}
		layout->addWidget(result);
		return result;
	};
}

void GuiTest::ShortcutDiscovery_gatherTooltips() {
	ShortcutDiscovery discovery{100ms};
	QSignalSpy activeSpy{&discovery, &ShortcutDiscovery::modifiersChanged};
	QSignalSpy labelsSpy{&discovery, &ShortcutDiscovery::labelsChanged};
	QWidget widget{};
	HUD hud{&widget};

	auto parent = new QWidget{&widget};
	auto layout = new QVBoxLayout{parent};
	layout->setContentsMargins(5, 5, 5, 5);
	layout->setSpacing(5);

	std::vector<ShortcutWidget*> widgets{};
	widgets.reserve(std::size(controls));
	std::transform(std::begin(controls), std::end(controls), std::back_inserter(widgets), widgetMaker(parent, layout));
	int height = 5;
	for (auto const& control : controls) {
		height += std::get<0>(control) + 5;
	}
	parent->resize(200, height);
	widget.resize(200, height * 2 / 3);

	widget.show();
	QVERIFY(QTest::qWaitForWindowExposed(&widget));

	{
		auto editor0 = discovery.beginHolderUpdate();
		auto editor = std::move(editor0);  // test move-ctor even if begin... uses RVO
		for (auto child : widgets) {
			editor.addHolder(child);
		}
	}

	QCOMPARE_EQ(discovery.holders().size(), std::size(widgets));

	QVERIFY(!discovery.isActive());

	hud.press(Qt::Key_Control);
	activeSpy.wait(200ms);
	QVERIFY(discovery.isActive());

	if (labelsSpy.size() < 1) labelsSpy.wait(100ms);
	QCOMPARE_EQ(labelsSpy.size(), 1);
	QCOMPARE_EQ(activeSpy.size(), 1);

	auto const previous = discovery.labels();
	{
		// "empty" update
		discovery.beginHolderUpdate();
	}
	auto const now = discovery.labels();
	auto actual = QList<std::pair<QPoint, QString>>{};
	actual.reserve(static_cast<qsizetype>(now.size()));
	std::transform(now.begin(), now.end(), std::back_inserter(actual),
	               [](ShortcutDiscovery::LabelInfo const& info) { return std::pair{info.origin, info.text}; });

	labelsSpy.wait(100ms);
	activeSpy.wait(100ms);
	QCOMPARE_EQ(labelsSpy.size(), 1);
	QCOMPARE_EQ(activeSpy.size(), 1);
	QCOMPARE_EQ(previous, now);
	QCOMPARE_EQ(actual, (QList{std::pair{QPoint{17, 49}, "Shift+E"_L1}, std::pair{QPoint{17, 108}, "Alt+Z"_L1},
	                           std::pair{QPoint{71, 108}, "Z"_L1}}));

	{
		auto editor = discovery.beginHolderUpdate();
		for (auto child : widgets) {
			editor.removeHolder(child);
		}
	}

	QCOMPARE_EQ(discovery.holders().size(), 0);

	if (labelsSpy.size() < 2) labelsSpy.wait(100ms);
	activeSpy.wait(100ms);
	QCOMPARE_EQ(labelsSpy.size(), 2);
	QCOMPARE_EQ(activeSpy.size(), 1);
}

void GuiTest::ShortcutDiscovery_toolButton() {
	using Support = HolderSupport<QToolButton>;

	QAction action{};
	QToolButton button{};
	action.setShortcuts(QList<QKeySequence>{Qt::CTRL | Qt::Key_A, Qt::CTRL | Qt::Key_B});
	button.setDefaultAction(&action);

	QCOMPARE(Support::keys(&button), (QList<QKeySequence>{Qt::CTRL | Qt::Key_A, Qt::CTRL | Qt::Key_B}));

	button.show();
	QVERIFY(QTest::qWaitForWindowExposed(&button));
	QVERIFY(Support::isEnabled(&button));

	action.setEnabled(false);
	QVERIFY(!Support::isEnabled(&button));

	action.setEnabled(true);
	button.setEnabled(false);
	QVERIFY(!Support::isEnabled(&button));

	button.setEnabled(true);
	button.setVisible(false);
	QVERIFY(!Support::isEnabled(&button));

	button.setVisible(true);
	QVERIFY(Support::isEnabled(&button));

	// call painting the tool tip label
	ToolTipLabel label{"Text", QPoint{}, nullptr};
	label.showNormal();
	QVERIFY(QTest::qWaitForWindowExposed(&label));
}
