// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <deque>
#include <stdexcept>
#include <webui.hpp>

namespace webui {
	namespace {
		std::deque<window_interface*>& mocks() {
			static std::deque<window_interface*> stg{};
			return stg;
		}

		std::function<void()>& wait_callback() {
			static std::function<void()> cb{[] {}};
			return cb;
		}
	}  // namespace
	window_interface::~window_interface() = default;

	window_interface::event::~event() = default;

	void window_interface::push_mock(window_interface* ptr) { mocks().push_back(ptr); }
	window_interface* window_interface::pull_mock() {
		if (mocks().empty()) {
			throw std::runtime_error("no more mocks");
		}
		auto next = mocks().front();
		mocks().pop_front();
		return next;
	}

	void set_wait_callback(std::function<void()> const& test) { wait_callback() = test; }
	void wait() { wait_callback()(); }
}  // namespace webui
