// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QApplication>
#include <QString>
#include <app/gui/Globals.hpp>
#include <app/main/MainWindow.hpp>
#include "options.hpp"

namespace quick_dra {
	int gui_tool(options const& opts) {
		gui::Globals globals{};
		globals.setConfig(opts.cfg_path, opts.tax_config_path);

		gui::MainWindow w{&globals};
		w.setWindowFilePath(QString::fromUtf8(as_sv(opts.cfg_path.u8string())));
		w.show();
		return QCoreApplication::exec();
	}
}  // namespace quick_dra

int gui_tool(int argc, char** argv) {
	QApplication a{argc, argv};

	QCoreApplication::setOrganizationName("midnightBITS");
	QCoreApplication::setApplicationName("Quick-DRA");

	a.setWindowIcon(QIcon{":/assets/icon"});

	try {
		return quick_dra::gui_tool(quick_dra::options::parse(::args::from_main(argc, argv)));
	} catch (quick_dra::exit_on_help const&) {
		return 0;
	}
}
