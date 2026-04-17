// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/std.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <quick_dra/base/str.hpp>
#include <quick_dra/conv/args_parser.hpp>
#include <quick_dra/gui/vfs.hpp>
#include <quick_dra/version.hpp>
#include <webui.hpp>

int gui_tool([[maybe_unused]] args::args_view const& arguments);

namespace quick_dra::gui::testing {
#define GMOCK_0(NAME, R) MOCK_METHOD(R, NAME, (), (override));
#define GMOCK_1(NAME, R, T1, A1) MOCK_METHOD(R, NAME, (T1), (override));
#define GMOCK_2(NAME, R, T1, A1, T2, A2) MOCK_METHOD(R, NAME, (T1, T2), (override));
	using ::testing::_;

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

	class mock_window : public ::webui::window_interface {
	public:
		std::map<std::string, std::function<void(::webui::window::event*)>> callbacks{};

		mock_window() {
			ON_CALL(*this, bind).WillByDefault([this](auto const& name, auto const& cb) {
				this->callbacks[as_str(name)] = cb;
			});
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

		WINDOW_0_ARG(GMOCK_0)
		WINDOW_1_ARG(GMOCK_1)
		WINDOW_2_ARG(GMOCK_2)
	};

#undef GMOCK_0
#undef GMOCK_1
#undef GMOCK_2

	TEST(gui_tool, window_setup) {
		mock_window mock{};
		::webui::window_interface::push_mock(&mock);

		using namespace std::literals;

		EXPECT_CALL(mock, bind("minimize"sv, _)).Times(1);
		EXPECT_CALL(mock, bind("maximize"sv, _)).Times(1);
		EXPECT_CALL(mock, bind("close_win"sv, _)).Times(1);
		EXPECT_CALL(mock, bind("get_version"sv, _)).Times(1);

		EXPECT_CALL(mock, minimize()).Times(1);
		EXPECT_CALL(mock, maximize()).Times(1);
		EXPECT_CALL(mock, close()).Times(1);

		EXPECT_CALL(mock, set_icon(_, "image/svg+xml"sv)).Times(1);
		EXPECT_CALL(mock, set_frameless(true)).Times(1);
		EXPECT_CALL(mock, set_transparent(true)).Times(1);
		EXPECT_CALL(mock, set_resizable(false)).Times(1);
		EXPECT_CALL(mock, set_size(800, 1200)).Times(1);
		EXPECT_CALL(mock, set_center()).Times(1);
		EXPECT_CALL(mock, set_file_handler(_)).Times(1);
		EXPECT_CALL(mock, show_wv("index.html"sv)).Times(1);

		std::string version_returned{};
		::webui::set_wait_callback([&mock, &version_returned] {
			mock.emit("minimize"sv, expect_get_window);
			mock.emit("maximize"sv, expect_get_window);
			mock.emit("close_win"sv, expect_get_window);
			mock.emit("get_version"sv, expect_return_string(version_returned));
		});

		EXPECT_EQ(gui_tool({}), 0);
		EXPECT_EQ(version_returned, version::ui);

		auto const& fs = virtual_filesystem::get_global();
		EXPECT_TRUE(fs.respond("index.html"));
		EXPECT_TRUE(fs.respond("index.js"));
		EXPECT_TRUE(fs.respond("favicon.svg"));
	}
}  // namespace quick_dra::gui::testing
