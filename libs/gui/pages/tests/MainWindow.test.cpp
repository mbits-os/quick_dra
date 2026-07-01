// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QPushButton>
#include <QSignalSpy>
#include <QTest>
#include <app/gui/Globals.hpp>
#include <app/main/MainWindow.hpp>
#include <app/pages/PersonelPage.hpp>
#include <quick_dra/version.hpp>
#include "PagesTest.hpp"
#include "TestPaths.hpp"
#include "ui_helpers.hpp"

#define QUICK_DRA_VERSION QUICK_DRA_VERSION_STR QUICK_DRA_VERSION_STABILITY QUICK_DRA_VERSION_BUILD_META

using namespace quick_dra::gui;
using namespace quick_dra;
namespace fs = std::filesystem;

static auto cliTestDataDir() { return fs::path{testing::cli_test_data}; }

#define QVERIFY_TITLE(TITLE) QCOMPARE_EQ(window.windowTitle(), QString{TITLE " - Quick-DRA " QUICK_DRA_VERSION})

void PagesTest::mainWindow() {
	TmpDir tmp{};
	platform::config_data_dir(testing::form_data_config);
	writeConfig(tmp.cwd(), cliTestDataDir());
	Globals globals{SettingsProvider::wrap([&tmp] { return openSettings(tmp.cwd()); })};
	globals.setConfig(tmp.cwd() / ".quick_dra.yaml"sv, std::nullopt, false);

	gui::MainWindow window{&globals};
	window.show();
	QVERIFY(QTest::qWaitForWindowExposed(&window));

	QVERIFY_TITLE("Podsumowanie");
	globals.stack().push<PersonelPage>();
	QVERIFY_TITLE("Dane osobowe");

	QEvent pce{QEvent::PaletteChange};
	QCloseEvent ce{};
	window.event(&pce);
	window.event(&ce);

	{
		QSettings settings = openSettings(tmp.cwd());
		settings.beginGroup("State");
		QVERIFY(settings.value("Geometry").isValid());
		QVERIFY(settings.value("WindowState").isValid());
	}

	PARENT_CONTEXT(window);
	ENSURE_CHILD(MessageBar, messageBar);
	ENSURE_CHILD(QPushButton, reloadButton);

	QSignalSpy configModifiedSpy{&globals, &Globals::configModifiedChanged};
	QSignalSpy visibleChangedSpy{messageBar, &MessageBar::visibleChanged};

	QVERIFY(!messageBar->isVisible());
	setFileContents(tmp.cwd() / ".quick_dra.yaml"sv,
	                fileContents(cliTestDataDir() / ".quick_dra.AB4123456_50671500000.quarter.yaml"sv));
	waitFor(configModifiedSpy, visibleChangedSpy);
	QVERIFY(messageBar->isVisible());

	reloadButton->click();
	waitFor(configModifiedSpy, visibleChangedSpy);
	QVERIFY(!messageBar->isVisible());
}

void PagesTest::createAndDeleteConfig() {
	TmpDir tmp{};
	// no writeConfigs here...
	Globals globals{SettingsProvider::wrap([&tmp] { return openSettings(tmp.cwd()); })};
	globals.setConfig(tmp.cwd() / ".quick_dra.yaml"sv, std::nullopt, false);

	gui::MainWindow window{&globals};
	window.show();
	QVERIFY(QTest::qWaitForWindowExposed(&window));

	PARENT_CONTEXT(window);
	ENSURE_CHILD(MessageBar, messageBar);
	ENSURE_CHILD(QPushButton, reloadButton);

	QSignalSpy configModifiedSpy{&globals, &Globals::configModifiedChanged};
	QSignalSpy visibleChangedSpy{messageBar, &MessageBar::visibleChanged};

	QVERIFY(!messageBar->isVisible());
	setFileContents(tmp.cwd() / ".not-the-quick_dra.yaml"sv,
	                fileContents(cliTestDataDir() / ".quick_dra.testing.yaml"sv));
	waitFor(configModifiedSpy, visibleChangedSpy);
	QVERIFY(!messageBar->isVisible());

	setFileContents(tmp.cwd() / ".quick_dra.yaml"sv, fileContents(cliTestDataDir() / ".quick_dra.testing.yaml"sv));
	waitFor(configModifiedSpy, visibleChangedSpy);
	QVERIFY(messageBar->isVisible());

	reloadButton->click();
	waitFor(configModifiedSpy, visibleChangedSpy);
	QVERIFY(!messageBar->isVisible());

	std::filesystem::remove(tmp.cwd() / ".quick_dra.yaml"sv);
	waitFor(configModifiedSpy, visibleChangedSpy);
	QVERIFY(messageBar->isVisible());
}
