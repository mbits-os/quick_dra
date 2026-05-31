// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <app/gui/PageHeader.hpp>
#include <app/gui/PageStack.hpp>

namespace quick_dra::gui {
	class Globals;
	class MainWindow : public QMainWindow {
		Q_OBJECT

	public:
		explicit MainWindow(Globals* globals, QWidget* parent = nullptr);

	private slots:
		void updateTitle();

	private:
		void setupUi();

		PageHeader* pageHeader{};
		QWidget* centralWidget{};
		QStackedWidget* stackedWidget{};
		QVBoxLayout* verticalLayout{};
		PageStack* pageStack{};
	};
}  // namespace quick_dra::gui
