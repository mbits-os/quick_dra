// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QObject>

class ControlsTest : public QObject {
	Q_OBJECT

private slots:
	void glyphProperties();
	void glyphPainter();
	void glyphSize();

	void PageScrollArea_simpleResize();
	void PageScrollArea_longText();

	void PanelButtonGroup_layout();
	void PanelButtonGroup_contents();
	void PanelButtonGroup_mouseMove();
	void PanelButtonGroup_mouseClick();
	void PanelButtonGroup_mouseClickWrongPlace();
	void PanelButtonGroup_mouseClickLeftRight();
	void PanelButtonGroup_mouseClickMoveAway();
	void PanelButtonGroup_mouseClickMoveAwayAndReturn();
	void PanelButtonGroup_mouseClickMoveOutside();
};
