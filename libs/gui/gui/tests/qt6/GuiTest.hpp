// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QObject>

class GuiTest : public QObject {
	Q_OBJECT

private slots:
	void PageHeader_movingShadow();
	void PageHeader_centeringTitle();
	void PageHeader_animate();
	void CurrentColor_changePaletteAtRuntime_data();
	void CurrentColor_changePaletteAtRuntime();
	void ShortcutDiscovery_shiftCtrlAltD();
	void ShortcutDiscovery_removeModifiers();
	void ShortcutDiscovery_removeKey();
	void ShortcutDiscovery_mouseButtons_data();
	void ShortcutDiscovery_mouseButtons();
	void ShortcutDiscovery_mouseWheel();
};
