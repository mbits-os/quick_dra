// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <app/main/MainWindow.hpp>
#include <format>
#include <quick_dra/version.hpp>

using namespace std::literals;

namespace quick_dra::gui {
	MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
		setWindowTitle(QString::fromStdString(std::format("{} {}", version::program, version::ui)));
	}
}  // namespace quick_dra::gui
