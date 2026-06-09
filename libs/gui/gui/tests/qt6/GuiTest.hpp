// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QObject>

class GuiTest : public QObject {
	Q_OBJECT

private slots:
	void PageHeader_movingShadow();
	void PageHeader_centeringTitle();
	void CurrentColor_changePaletteAtRuntime();
};
