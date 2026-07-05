// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QEvent>
#include <concepts>

namespace quick_dra::gui {
	template <std::derived_from<QEvent> T>
	inline QEvent::Type typeOf() noexcept {
		static auto id = static_cast<QEvent::Type>(QEvent::registerEventType());
		return id;
	}

	template <std::derived_from<QEvent> T>
	inline T* user_event_cast(QEvent* event) noexcept {
		if (event->type() == typeOf<T>()) return static_cast<T*>(event);
		return nullptr;
	}

	class PageFocusEvent : public QEvent {
	public:
		explicit PageFocusEvent(bool hasFocus) : QEvent{typeOf<PageFocusEvent>()}, value{hasFocus} {}

		bool hasFocus() const noexcept { return value; }

	private:
		bool value;
	};
}  // namespace quick_dra::gui
