// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QApplication>
#include <string>
#include <string_view>
#include <vector>

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
}  // namespace quick_dra::gui::testing
