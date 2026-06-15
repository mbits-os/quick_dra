// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QObject>

class PagesTest : public QObject {
	Q_OBJECT

private slots:
	void mainWindow();
	void pushPage_data();
	void pushPage();
};
