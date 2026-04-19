// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

// For definitions copied from true webui,
// Copyright(c) 2020 - 2026 Hassan Draga.

#pragma once

#include <functional>
#include <string_view>

namespace webui {
#define WINDOW_0_ARG(X) \
	X(minimize, void)   \
	X(maximize, void)   \
	X(close, void)      \
	X(set_center, void) \
	X(get_hwnd, void*)

#define WINDOW_1_ARG(X)                             \
	X(set_frameless, void, bool, value)             \
	X(set_transparent, void, bool, value)           \
	X(set_resizable, void, bool, value)             \
	X(set_file_handler, void, vfs_handler, handler) \
	X(show_wv, void, std::string_view, uri)         \
	X(run, void, std::string_view, script)

#define WINDOW_2_ARG(X)                                                     \
	X(bind, void, const std::string_view, element, event::callback_t, func) \
	X(set_icon, void, std::string_view, data, std::string_view, type)       \
	X(set_size, void, int, width, int, height)

#define EVENT_0_ARG(X) X(get_window, window_interface&)
#define EVENT_1_ARG(X) X(return_string, void, std::string_view, result)

#define ABSTRACT_0(NAME, R) virtual R NAME() = 0;
#define ABSTRACT_1(NAME, R, T1, A1) virtual R NAME(T1 A1) = 0;
#define ABSTRACT_2(NAME, R, T1, A1, T2, A2) virtual R NAME(T1 A1, T2 A2) = 0;

	class window_interface {
	public:
		virtual ~window_interface();

		using vfs_handler = const void* (*)(const char* filename, int* length);

		class event {
		public:
			using callback_t = std::function<void(event*)>;

			virtual ~event();

			EVENT_0_ARG(ABSTRACT_0)
			EVENT_1_ARG(ABSTRACT_1)
		};

		template <typename T>
		void bind(const std::string_view element, T* instance, void (T::*method)(event*)) {
			bind(element, [instance, method](event* e) { (instance->*method)(e); });
		}

		WINDOW_0_ARG(ABSTRACT_0)
		WINDOW_1_ARG(ABSTRACT_1)
		WINDOW_2_ARG(ABSTRACT_2)

		static void push_mock(window_interface*);
		static window_interface* pull_mock();
	};
#undef ABSTRACT_0
#undef ABSTRACT_1
#undef ABSTRACT_2

#define PROXY_0(NAME, R) \
	R NAME() override { return proxy->NAME(); }
#define PROXY_1(NAME, R, T1, A1) \
	R NAME(T1 A1) override { return proxy->NAME(A1); }
#define PROXY_2(NAME, R, T1, A1, T2, A2) \
	R NAME(T1 A1, T2 A2) override { return proxy->NAME(A1, A2); }

	class window : public window_interface {
	public:
		WINDOW_0_ARG(PROXY_0)
		WINDOW_1_ARG(PROXY_1)
		WINDOW_2_ARG(PROXY_2)

	private:
		window_interface* proxy = pull_mock();
	};

	void set_wait_callback(std::function<void()> const& test);
	void wait();
}  // namespace webui
