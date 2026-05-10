// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QFont>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QList>
#include <QMainWindow>
#include <QSlider>
#include <QStackedWidget>
#include <QWidget>
#include <app/gui/PageHeader.hpp>
#include <app/gui/PageStack.hpp>
#include <app/utils/FormData.hpp>
#include <quick_dra/docs/forms.hpp>
#include <vector>

#undef emit
#include <quick_dra/models/types.hpp>

namespace quick_dra::gui {
	class MainWindow;
}

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
		void setupUi();
		void storePosition();
		void restorePosition();
		void updateStyles();

		PageHeader* pageHeader{};
		QWidget* centralWidget{};
		QStackedWidget* stackedWidget{};
		QVBoxLayout* verticalLayout{};
		QWidget* messageBar{};
		PageStack* pageStack{};
	};
}  // namespace quick_dra::gui
