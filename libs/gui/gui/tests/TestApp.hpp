// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <fmt/format.h>
#include <QApplication>
#include <app/gui/Globals.hpp>
#include <app/gui/PageHeader.hpp>
#include <app/gui/PageStack.hpp>
#include <app/gui/PagedWidget.hpp>
#include <app/utils/str.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include "EnvPaths.hpp"
#include "ui_helpers.hpp"

using namespace std::literals;

namespace quick_dra::gui::testing {
	struct TestAppArgs {
		std::vector<std::string> args{};
		std::vector<char*> argv_{};
		int argc_{};

		TestAppArgs(std::vector<std::string>&& argv) : args{std::move(argv)} {
			argv_.clear();
			argv_.reserve(args.size() + 1);
			std::transform(args.begin(), args.end(), std::back_inserter(argv_), [](auto& s) { return s.data(); });
			argv_.push_back(nullptr);
			argc_ = static_cast<int>(args.size());
		}

		char** argv() noexcept { return argv_.data(); }
		int& argc() noexcept { return argc_; }
	};

	struct TestApp : public TestAppArgs, public QApplication {
		TestApp() : TestApp(std::vector{"test"s, "-platform"s, "offscreen"s}) {}
		TestApp(std::vector<std::string>&& argv)
		    : TestAppArgs{std::move(argv)}, QApplication{this->argc(), this->argv()} {}
	};

	struct Env {
		TmpDir tmp_dir{};
		TestApp app{};
		Globals globals{SettingsProvider::wrap([self = this] { return openSettings(self->tmp_dir.cwd()); })};
		QStackedWidget stackedWidget{};
		PageHeader* header = new PageHeader{&stackedWidget};
		PageStack* stack = new PageStack{header, &stackedWidget};

		bool formDirty() const noexcept { return stack->formDirty(); }
		bool formValid() const noexcept { return stack->formValid(); }
		bool topMost() const noexcept { return stack->topMost(); }

		Env() {
			platform::config_data_dir(form_data_config);
			header->setObjectName("PageHeader");
			globals.setStack(stack);
		}
	};
}  // namespace quick_dra::gui::testing
