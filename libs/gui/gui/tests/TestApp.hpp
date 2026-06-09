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

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

using namespace std::literals;

namespace quick_dra::gui::testing {
#ifdef WIN32
	inline char* mkdtemp(char* buffer) {
		_mktemp(buffer);
		auto const path = std::filesystem::path{as_u8v(std::string_view{buffer})};
		create_directories(path);
		return buffer;
	}
#endif

	struct TmpDir {
		TmpDir() { std::filesystem::current_path(dirname_); }
		TmpDir(TmpDir const&) = delete;
		TmpDir(TmpDir&&) = delete;
		~TmpDir() {
			std::error_code ec{};
			std::filesystem::current_path(cwd_);
			std::filesystem::remove_all(dirname_, ec);
		}

		[[nodiscard]] std::filesystem::path const& cwd() const noexcept { return dirname_; };

	private:
		std::filesystem::path make_temp_dir() {
			auto const path = std::filesystem::temp_directory_path() / fmt::format("dirXXXXXX");
			auto name = as_str(path.u8string());
			return as_u8v(std::string_view{mkdtemp(name.data())});
		}

		std::filesystem::path dirname_{make_temp_dir()};
		std::filesystem::path cwd_{std::filesystem::current_path()};
	};

	inline std::string file_contents(std::filesystem::path const& path) {
		auto file = std::ifstream{path};
		auto str = std::ostringstream{};
		str << file.rdbuf();
		return std::move(str).str();
	}

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
		Globals globals{SettingsProvider::wrap([self = this] { return self->provideSettings(); })};
		QStackedWidget stackedWidget{};
		PageHeader* header = new PageHeader{&stackedWidget};
		PageStack* stack = new PageStack{header, &stackedWidget};

		QSettings provideSettings() const {
			return {QString::fromUtf8(as_sv((tmp_dir.cwd() / "config.ini").generic_u8string())), QSettings::IniFormat};
		}

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
