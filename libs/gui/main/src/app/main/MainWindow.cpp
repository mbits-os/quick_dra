// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <app/gui/Globals.hpp>
#include <app/gui/PagedWidget.hpp>
#include <app/main/MainWindow.hpp>
#include <format>
#include <quick_dra/version.hpp>

using namespace std::literals;

namespace quick_dra::gui {
	MainWindow::MainWindow(Globals* globals, QWidget* parent) : QMainWindow(parent) {
		setupUi();
		setWindowTitle(QString::fromStdString(std::format("{} {}", version::program, version::ui)));
		globals->setStack(pageStack);
	}

	void MainWindow::updateTitle() {
		auto const page = pageStack->page();
		auto const title = page ? page->windowTitle() : QString{};
		auto const prefix = title.isEmpty() ? QString{} : QString{"%1 - "}.arg(title);
		setWindowTitle(
		    QString::fromStdString(std::format("{}{} {}", prefix.toStdString(), version::program, version::ui)));
	}
}  // namespace quick_dra::gui
