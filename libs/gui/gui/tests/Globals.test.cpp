// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <gtest/gtest.h>
#include "TestApp.hpp"

namespace quick_dra::gui::testing {
	struct GlobEnv : Env {
		size_t configurationChanged{};
		size_t identifierChanged{};
		size_t formSetChanged{};
		size_t configModifiedChanged{};

		static auto cliTestDataDir() { return std::filesystem::path{cli_test_data}; }
		GlobEnv() {
			{
				auto in_file = std::ifstream{cliTestDataDir() / ".quick_dra.AB4123456_50671500000.quarter.yaml"sv};
				auto out_file = std::ofstream{tmp_dir.cwd() / ".quick_dra.yaml"sv, std::ios::binary | std::ios::out};
				out_file << in_file.rdbuf();
			}
			observeSignal(&Globals::configurationChanged, configurationChanged);
			observeSignal(&Globals::identifierChanged, identifierChanged);
			observeSignal(&Globals::formSetChanged, formSetChanged);
			observeSignal(&Globals::configModifiedChanged, configModifiedChanged);
		}

		void observeSignal(void (Globals::*signal)(), size_t& counter) {
			QObject::connect(&globals, signal, [&counter] { ++counter; });
		}

		void observeSignal(void (Globals::*signal)(bool), size_t& counter) {
			QObject::connect(&globals, signal, [&counter] { ++counter; });
		}
	};

#define EXPECT_CALLS(CONFIGURATION_COUNT, IDENTIFIER_COUNT, FORMSET_COUNT, CONFIGMODIFIED_COUNT) \
	EXPECT_EQ(env.configurationChanged, CONFIGURATION_COUNT);                                    \
	EXPECT_EQ(env.identifierChanged, IDENTIFIER_COUNT);                                          \
	EXPECT_EQ(env.formSetChanged, FORMSET_COUNT);                                                \
	EXPECT_EQ(env.configModifiedChanged, CONFIGMODIFIED_COUNT);                                  \
	env.configurationChanged = env.identifierChanged = env.formSetChanged = env.configModifiedChanged = 0

	TEST(Globals, config) {
		GlobEnv env{};
		EXPECT_CALLS(0, 0, 0, 0);

		env.globals.setConfig(env.tmp_dir.cwd() / ".quick_dra.yaml"sv, std::nullopt, false);
		EXPECT_CALLS(1, 0, 1, 0);

		auto payer = *env.globals.data().cfg.payer;
		payer.first_name = "First"sv;
		payer.last_name = "Last"sv;

		env.globals.storePayer(payer);
		EXPECT_CALLS(1, 0, 1, 0);

		env.globals.storeInsured(env.globals.data().cfg.insured->size(), {});
		EXPECT_CALLS(1, 0, 1, 0);

		env.globals.storeInsured(env.globals.data().cfg.insured->size(), {});
		EXPECT_CALLS(1, 0, 1, 0);

		env.globals.storeInsured(env.globals.data().cfg.insured->size(), {});
		EXPECT_CALLS(1, 0, 1, 0);

		env.globals.storeInsured(0, {});
		EXPECT_CALLS(1, 0, 1, 0);

		env.globals.storeInsured(env.globals.data().cfg.insured->size() + 1, {});
		EXPECT_CALLS(0, 0, 0, 0);

		env.globals.removeInsured({0, env.globals.data().cfg.insured->size() - 1});
		EXPECT_CALLS(1, 0, 1, 0);
	}

	TEST(Globals, configModified) {
		GlobEnv env{};
		EXPECT_FALSE(env.globals.configModified());

		env.globals.setConfig(env.tmp_dir.cwd() / ".quick_dra.yaml"sv, std::nullopt, false);
		EXPECT_CALLS(1, 0, 1, 0);

		auto const almost =
		    QString::fromUtf8(as_sv((env.tmp_dir.cwd() / "name"sv / ".."sv / ".quick_dra.yaml"sv).u8string()));
		env.globals.observedFileChanged(almost);  // same last write time
		EXPECT_CALLS(0, 0, 0, 0);

		{
			auto out_file = std::ofstream{env.tmp_dir.cwd() / ".quick_dra.yaml"sv, std::ios::binary | std::ios::out};
			out_file << "# nothing\n";
		}

		env.globals.observedFileChanged(almost);
		EXPECT_CALLS(0, 0, 0, 1);
		EXPECT_TRUE(env.globals.configModified());

		env.globals.reloadConfig();
		EXPECT_CALLS(1, 0, 1, 1);
		EXPECT_FALSE(env.globals.configModified());
	}

	TEST(Globals, identifier) {
		GlobEnv env{};
		env.globals.storeIdentifier({.index = 1, .date = 2020y / 5, .isOverriden = true});
		EXPECT_CALLS(0, 1, 1, 0);
#ifdef WIN32
		static constexpr auto expected_settings = "[Settings]\nYearMonth=2020/5\nReportIndex=1\n"sv;
#else
		static constexpr auto expected_settings = "[Settings]\nReportIndex=1\nYearMonth=2020/5\n"sv;
#endif
		EXPECT_EQ(file_contents(env.tmp_dir.cwd() / "config.ini"), expected_settings);

		Globals globals2{SettingsProvider::wrap([&env] { return env.provideSettings(); })};
		EXPECT_EQ(globals2.reportId().index, 1);
		EXPECT_EQ(globals2.reportId().date, 2020y / 5);
		EXPECT_TRUE(globals2.reportId().isOverriden);

		env.globals.storeIdentifier({.index = 1, .date = 2020y / 5, .isOverriden = false});
		EXPECT_CALLS(0, 1, 1, 0);
		EXPECT_EQ(file_contents(env.tmp_dir.cwd() / "config.ini"), "[Settings]\nReportIndex=1\n"sv);
	}

	TEST(Globals, settingsProvider) {
		{
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
