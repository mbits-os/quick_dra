// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QEvent>
#include <QSettings>
#include <app/gui/Globals.hpp>
#include <app/main/MainWindow.hpp>
#include <app/pages/HomePage.hpp>
#include <print>
#include <quick_dra/version.hpp>

using namespace std::literals;

namespace quick_dra::gui {
	MainWindow::MainWindow(Globals* globals, QWidget* parent) : QMainWindow(parent) {
		setupUi();
		setWindowTitle(QString::fromStdString(fmt::format("{} {}", version::program, version::ui)));
		globals->setStack(pageStack);
		pageStack->push<HomePage>();

		QObject::connect(globals, &Globals::configModifiedChanged, messageBar, &QWidget::setVisible);
	}

	void MainWindow::closeEvent(QCloseEvent* event) {
		QMainWindow::closeEvent(event);
		storePosition();
	}

	bool MainWindow::event(QEvent* event) {
		if (event->type() == QEvent::PaletteChange) {
			updateStyles();
		}
		return QMainWindow::event(event);
	}

	void MainWindow::updateTitle() {
		auto const page = pageStack->page();
		auto const title = page ? page->windowTitle() : QString{};
		auto const prefix = title.isEmpty() ? QString{} : QString{"%1 - "}.arg(title);
		setWindowTitle(
		    QString::fromStdString(fmt::format("{}{} {}", prefix.toStdString(), version::program, version::ui)));
	}

	void MainWindow::configModified(bool modified) { messageBar->setVisible(modified); }

	void MainWindow::reloadConfig() {
		pageStack->navigateHomeForReload();
		pageStack->globals().reloadConfig();
	}

	void MainWindow::storePosition() {
		QSettings settings{};
		settings.beginGroup("State");
		settings.setValue("Geometry", saveGeometry());
		settings.setValue("WindowState", saveState());
		settings.endGroup();
	}

	void MainWindow::restorePosition() {
		QSettings settings{};
		settings.beginGroup("State");
		if (!restoreGeometry(settings.value("Geometry").toByteArray())) {
			resize(450, 450);
		}
		auto const savedState = windowState();
		if (!restoreState(settings.value("WindowState").toByteArray())) {
			setWindowState(savedState);
		}
		settings.endGroup();
	}
}  // namespace quick_dra::gui
