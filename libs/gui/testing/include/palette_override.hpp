// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QApplication>
#include <QColor>
#include <QPalette>
#include <QWidget>

struct PaletteOverride {
	QColor window;
	QColor windowText;

	void setPalette(auto* target) const {
		auto const lightness = (window.toHsl().lightnessF() + windowText.toHsl().lightnessF() * 2) / 3;
		auto const disabled = QColor::fromRgbF(lightness, lightness, lightness, windowText.alphaF()).toRgb();

		auto palette = target->palette();
		palette.setColor(QPalette::Normal, QPalette::Window, window);
		palette.setColor(QPalette::Normal, QPalette::WindowText, windowText);
		palette.setColor(QPalette::Disabled, QPalette::Window, window);
		palette.setColor(QPalette::Disabled, QPalette::WindowText, disabled);
		target->setPalette(palette);
	}

	void install(QWidget* widget) const {
		setPalette(qApp);
		setPalette(widget);
	}
};
