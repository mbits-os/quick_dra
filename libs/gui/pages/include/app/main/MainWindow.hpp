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
	class MessageBar : public QWidget {
		Q_OBJECT

	public:
		using QWidget::QWidget;

		void setVisible(bool value) override;

	signals:
		void visibleChanged(bool value);
	};

	class MainWindow : public QMainWindow {
		Q_OBJECT

	public:
		explicit MainWindow(Globals* globals, QWidget* parent = nullptr);

		void closeEvent(QCloseEvent*) override;
		bool event(QEvent*) override;

	private slots:
		void updateTitle();
		void reloadConfig();
		void discoveryModifiers(Qt::KeyboardModifiers);

	private:
		void setupUi(Globals* globals);
		void storePosition(Globals* globals);
		void restorePosition(Globals* globals);
		void updateStyles();

		PageHeader* pageHeader{};
		QWidget* centralWidget{};
		QStackedWidget* stackedWidget{};
		QVBoxLayout* verticalLayout{};
		MessageBar* messageBar{};
		PageStack* pageStack{};
	};
}  // namespace quick_dra::gui
