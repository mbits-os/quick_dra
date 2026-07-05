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

	void DevicePixelScale();
	void PanelButtonGroup_layout();
	void PanelButtonGroup_contents();
	void PanelButtonGroup_shortcuts();
	void PanelButtonGroup_mouseMove();
	void PanelButtonGroup_mouseClick();
	void PanelButtonGroup_mouseClickWrongPlace();
	void PanelButtonGroup_mouseClickLeftRight();
	void PanelButtonGroup_mouseClickMoveAway();
	void PanelButtonGroup_mouseClickMoveAwayAndReturn();
	void PanelButtonGroup_mouseClickMoveOutside();

	void Forms_access();
	void Forms_lineEdit();
	void Forms_documentComboBox();
	void Forms_tableView();

	void SplitSpaces_data();
	void SplitSpaces();
	void InlineMarkdown_data();
	void InlineMarkdown();
	void MeasureText_data();
	void MeasureText();
	void PaintMarkdown();
	void FindSmallestRect();
};
