// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/std.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <initializer_list>
#include <quick_dra/base/str.hpp>
#include <quick_dra/conv/args_parser.hpp>
#include <quick_dra/gui/vfs.hpp>
#include <quick_dra/version.hpp>
#include <span>
#include <webui.hpp>

int gui_tool([[maybe_unused]] args::args_view const& arguments);

namespace quick_dra::gui::testing {
#define GMOCK_0(NAME, R) MOCK_METHOD(R, NAME, (), (override));
#define GMOCK_1(NAME, R, T1, A1) MOCK_METHOD(R, NAME, (T1), (override));
#define GMOCK_2(NAME, R, T1, A1, T2, A2) MOCK_METHOD(R, NAME, (T1, T2), (override));

	using ::testing::_;
	using ::testing::IsFalse;
	using ::testing::IsTrue;

	struct arglist {
		std::vector<std::string> stg{};
		std::vector<char*> argv{};

		arglist(std::initializer_list<std::string_view> items) {
			stg.reserve(items.size() + 1);
			stg.push_back("qdra-qui"s);
			std::transform(items.begin(), items.end(), std::back_inserter(stg), conv<std::string, std::string_view>);

			argv.reserve(stg.size() + 1);
			std::transform(stg.begin(), stg.end(), std::back_inserter(argv), [](auto& arg) { return arg.data(); });
			argv.push_back(nullptr);
		}

		args::args_view as_args() { return args::from_main({static_cast<unsigned>(stg.size()), argv.data()}); };
	};

	class mock_event : public ::webui::window_interface::event {
	public:
		using window_interface = ::webui::window_interface;

		window_interface* target;
		mock_event(window_interface* target) : target{target} {
			ON_CALL(*this, get_window()).WillByDefault([this] -> window_interface& { return *this->target; });
		}

		EVENT_0_ARG(GMOCK_0)
		EVENT_1_ARG(GMOCK_1)
	};

	static void expect_get_window(mock_event& event) { EXPECT_CALL(event, get_window()).Times(1); }
	static std::function<void(mock_event&)> expect_return_string(std::string& stg) {
		return [&stg](mock_event& event) {
			EXPECT_CALL(event, return_string(_)).Times(1).WillOnce([&stg](auto const& value) { stg = value; });
		};
	}
	static void expect_return_any_string(mock_event& event) { EXPECT_CALL(event, return_string(_)).Times(1); }

	class mock_window : public ::webui::window_interface {
	public:
		std::map<std::string, std::function<void(::webui::window::event*)>> callbacks{};

		mock_window() {
			ON_CALL(*this, bind).WillByDefault([this](auto const& name, auto const& cb) {
				this->callbacks[as_str(name)] = cb;
			});

			EXPECT_CALL(*this, bind("minimize"sv, _)).Times(1);
			EXPECT_CALL(*this, bind("maximize"sv, _)).Times(1);
			EXPECT_CALL(*this, bind("close_win"sv, _)).Times(1);
			EXPECT_CALL(*this, bind("get_config"sv, _)).Times(1);

#ifdef _WIN32
			EXPECT_CALL(*this, get_hwnd()).Times(1);
#endif

			EXPECT_CALL(*this, set_size(800, 1200)).Times(1);
			EXPECT_CALL(*this, set_center()).Times(1);
			EXPECT_CALL(*this, set_file_handler(_)).Times(1);

			EXPECT_CALL(*this, show_wv("index.html"sv)).Times(1);
		}

		void expect_frameless(int n = 1) {
			EXPECT_CALL(*this, set_frameless(IsTrue())).Times(n);
			EXPECT_CALL(*this, set_transparent(IsTrue())).Times(n);
			EXPECT_CALL(*this, set_resizable(IsFalse())).Times(n);
		}

		void emit(std::string_view name, std::function<void(mock_event&)> const& setup_expectations) {
			mock_event event{this};
			setup_expectations(event);
			auto it = callbacks.find(as_str(name));
			if (it == callbacks.end()) {
				return;
			}
			it->second(&event);
		}

		void emit_all() {
			EXPECT_CALL(*this, minimize()).Times(1);
			EXPECT_CALL(*this, maximize()).Times(1);
			EXPECT_CALL(*this, close()).Times(1);

			::webui::set_wait_callback([&mock = *this] {
				mock.emit("minimize"sv, expect_get_window);
				mock.emit("maximize"sv, expect_get_window);
				mock.emit("close_win"sv, expect_get_window);
				mock.emit("get_config"sv, expect_return_any_string);
			});
		}

		WINDOW_0_ARG(GMOCK_0)
		WINDOW_1_ARG(GMOCK_1)
		WINDOW_2_ARG(GMOCK_2)
	};

#undef GMOCK_0
#undef GMOCK_1
#undef GMOCK_2

	TEST(gui_tool, default_run) {
		mock_window mock{};
		::webui::window_interface::push_mock(&mock);

		using namespace std::literals;

		mock.expect_frameless();
		EXPECT_CALL(mock, set_wv_devtools_available(_)).Times(0);

		mock.emit_all();

		EXPECT_EQ(gui_tool({}), 0);

		auto const& fs = virtual_filesystem::get_global();
		EXPECT_TRUE(fs.respond("index.html", {}));
		EXPECT_TRUE(fs.respond("index.js", {}));
		EXPECT_TRUE(fs.respond("favicon.svg", {}));
	}

	TEST(gui_tool, no_transparency) {
		mock_window mock{};
		::webui::window_interface::push_mock(&mock);

		using namespace std::literals;

		mock.expect_frameless(0);
		EXPECT_CALL(mock, set_wv_devtools_available(_)).Times(0);

		mock.emit_all();

		EXPECT_EQ(gui_tool(arglist{"--no-transparent"sv}.as_args()), 0);

		auto const& fs = virtual_filesystem::get_global();
		EXPECT_TRUE(fs.respond("index.html", {}));
		EXPECT_TRUE(fs.respond("index.js", {}));
		EXPECT_TRUE(fs.respond("favicon.svg", {}));
	}

	TEST(gui_tool, set_devtool_to_true) {
		mock_window mock{};
		::webui::window_interface::push_mock(&mock);

		using namespace std::literals;

		mock.expect_frameless();
		EXPECT_CALL(mock, set_wv_devtools_available(IsTrue())).Times(1);

		mock.emit_all();

		EXPECT_EQ(gui_tool(arglist{"--devtools"sv}.as_args()), 0);

		auto const& fs = virtual_filesystem::get_global();
		EXPECT_TRUE(fs.respond("index.html", {}));
		EXPECT_TRUE(fs.respond("index.js", {}));
		EXPECT_TRUE(fs.respond("favicon.svg", {}));
	}

	TEST(gui_tool, use_debug_app_dir) {
		mock_window mock{};
		::webui::window_interface::push_mock(&mock);

		using namespace std::literals;

		mock.expect_frameless();
		EXPECT_CALL(mock, set_wv_devtools_available(IsTrue())).Times(1);

		mock.emit_all();

		EXPECT_EQ(gui_tool(arglist{"--dbg-app"sv, "."sv}.as_args()), 0);
	}
}  // namespace quick_dra::gui::testing
