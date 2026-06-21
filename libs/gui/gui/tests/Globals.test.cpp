// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <gtest/gtest.h>
#include "TestApp.hpp"

namespace fs = std::filesystem;
namespace quick_dra::gui::testing {
	struct GlobEnv : Env {
		QSignalSpy configModifiedSpy{&globals, &Globals::configModifiedChanged};

		static auto cliTestDataDir() { return fs::path{cli_test_data}; }
		GlobEnv() {
			fs::copy_file(cliTestDataDir() / ".quick_dra.AB4123456_50671500000.quarter.yaml"sv,
			              tmp_dir.cwd() / ".quick_dra.yaml"sv, fs::copy_options::overwrite_existing);
		}
	};

#define EXPECT_SIGNAL(SPY) EXPECT_TRUE(takeFirst(SPY))
#define EXPECT_NO_SIGNAL(SPY)     \
	do {                          \
		SPY.wait(100ms);          \
		EXPECT_TRUE(SPY.empty()); \
	} while (false)

	TEST(Globals, config) {
		GlobEnv env{};

		QSignalSpy configurationSpy{&env.globals, &Globals::configurationChanged};
		QSignalSpy formSetSpy{&env.globals, &Globals::formSetChanged};

		env.globals.setConfig(env.tmp_dir.cwd() / ".quick_dra.yaml"sv, std::nullopt, false);
		EXPECT_SIGNAL(configurationSpy);
		EXPECT_SIGNAL(formSetSpy);

		auto payer = *env.globals.data().cfg.payer;
		payer.first_name = "First"sv;
		payer.last_name = "Last"sv;

		env.globals.storePayer(payer);
		EXPECT_SIGNAL(configurationSpy);
		EXPECT_SIGNAL(formSetSpy);

		env.globals.storeInsured(env.globals.data().cfg.insured->size(), {});
		EXPECT_SIGNAL(configurationSpy);
		EXPECT_SIGNAL(formSetSpy);

		env.globals.storeInsured(env.globals.data().cfg.insured->size(), {});
		EXPECT_SIGNAL(configurationSpy);
		EXPECT_SIGNAL(formSetSpy);

		env.globals.storeInsured(env.globals.data().cfg.insured->size(), {});
		EXPECT_SIGNAL(configurationSpy);
		EXPECT_SIGNAL(formSetSpy);

		env.globals.storeInsured(0, {});
		EXPECT_SIGNAL(configurationSpy);
		EXPECT_SIGNAL(formSetSpy);

		env.globals.storeInsured(env.globals.data().cfg.insured->size() + 1, {});
		EXPECT_NO_SIGNAL(configurationSpy);
		EXPECT_NO_SIGNAL(formSetSpy);

		env.globals.removeInsured({0, env.globals.data().cfg.insured->size() - 1});
		EXPECT_SIGNAL(configurationSpy);
		EXPECT_SIGNAL(formSetSpy);
	}

	TEST(Globals, configModified) {
		GlobEnv env{};
		EXPECT_FALSE(env.globals.configModified());

		QSignalSpy configurationSpy{&env.globals, &Globals::configurationChanged};
		QSignalSpy formSetSpy{&env.globals, &Globals::formSetChanged};
		QSignalSpy configModifiedSpy{&env.globals, &Globals::configModifiedChanged};

		env.globals.setConfig(env.tmp_dir.cwd() / ".quick_dra.yaml"sv, std::nullopt, false);
		EXPECT_SIGNAL(configurationSpy);
		EXPECT_SIGNAL(formSetSpy);
		EXPECT_NO_SIGNAL(configModifiedSpy);

		auto const almost =
		    QString::fromUtf8(as_sv((env.tmp_dir.cwd() / "name"sv / ".."sv / ".quick_dra.yaml"sv).u8string()));
		env.globals.observedFileChanged(almost);  // same last write time
		EXPECT_NO_SIGNAL(configurationSpy);
		EXPECT_NO_SIGNAL(formSetSpy);
		EXPECT_NO_SIGNAL(configModifiedSpy);

		{
			auto out_file = std::ofstream{env.tmp_dir.cwd() / ".quick_dra.yaml"sv, std::ios::binary | std::ios::out};
			out_file << "# nothing\n";
		}

		env.globals.observedFileChanged(almost);
		EXPECT_SIGNAL(configModifiedSpy);
		EXPECT_NO_SIGNAL(configurationSpy);
		EXPECT_NO_SIGNAL(formSetSpy);
		EXPECT_TRUE(env.globals.configModified());

		env.globals.reloadConfig();
		EXPECT_SIGNAL(configurationSpy);
		EXPECT_SIGNAL(formSetSpy);
		EXPECT_SIGNAL(configModifiedSpy);
		EXPECT_FALSE(env.globals.configModified());
	}

	TEST(Globals, identifier) {
		GlobEnv env{};
		QSignalSpy identifierSpy{&env.globals, &Globals::identifierChanged};
		QSignalSpy formSetSpy{&env.globals, &Globals::formSetChanged};

		env.globals.storeIdentifier({.index = 1, .date = 2020y / 5, .isOverriden = true});
		EXPECT_SIGNAL(identifierSpy);
		EXPECT_SIGNAL(formSetSpy);
#ifdef WIN32
		static constexpr auto expected_settings = "[Settings]\nYearMonth=2020/5\nReportIndex=1\n"sv;
#else
		static constexpr auto expected_settings = "[Settings]\nReportIndex=1\nYearMonth=2020/5\n"sv;
#endif
		EXPECT_EQ(fileContents(env.tmp_dir.cwd() / "config.ini"), expected_settings);

		Globals globals2{SettingsProvider::wrap([&env] { return openSettings(env.tmp_dir.cwd()); })};
		EXPECT_EQ(globals2.reportId().index, 1);
		EXPECT_EQ(globals2.reportId().date, 2020y / 5);
		EXPECT_TRUE(globals2.reportId().isOverriden);

		env.globals.storeIdentifier({.index = 1, .date = 2020y / 5, .isOverriden = false});
		EXPECT_SIGNAL(identifierSpy);
		EXPECT_SIGNAL(formSetSpy);
		EXPECT_EQ(fileContents(env.tmp_dir.cwd() / "config.ini"), "[Settings]\nReportIndex=1\n"sv);
	}

	TEST(Globals, settingsProvider) {
		{
			TestApp app{};
			Globals globals{};
			auto settings = globals.createSettings();
			ASSERT_EQ(settings.format(), QSettings::NativeFormat);
			ASSERT_EQ(settings.scope(), QSettings::UserScope);
		}
		{
			GlobEnv env{};
			auto settings = env.globals.createSettings();
			ASSERT_EQ(settings.format(), QSettings::IniFormat);
			ASSERT_EQ(settings.scope(), QSettings::UserScope);
			ASSERT_EQ(settings.fileName(),
			          QString::fromUtf8(as_sv((env.tmp_dir.cwd() / "config.ini").generic_u8string())));
		}
	}
}  // namespace quick_dra::gui::testing
