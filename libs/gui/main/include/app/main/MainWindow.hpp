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

		void closeEvent(QCloseEvent*) override;
		bool event(QEvent*) override;

	private slots:
		void updateTitle();
		void configModified(bool);
		void reloadConfig();

	private:
		void setupUi(Globals* globals);
		void storePosition(Globals* globals);
		void restorePosition(Globals* globals);
		void updateStyles();

		PageHeader* pageHeader{};
		QWidget* centralWidget{};
		QStackedWidget* stackedWidget{};
		QVBoxLayout* verticalLayout{};
		QWidget* messageBar{};
		PageStack* pageStack{};
	};
}  // namespace quick_dra::gui
