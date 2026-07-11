// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QApplication>
#include <QDebug>
#include <QKeyEvent>
#include <app/gui/Globals.hpp>
#include <app/gui/ShortcutDiscovery.hpp>
#include <array>
#include <utility>

namespace quick_dra::gui {
	namespace {
		static constexpr auto mods = std::array{
		    std::pair{Qt::Key_Shift, Qt::ShiftModifier},
		    std::pair{Qt::Key_Control, Qt::ControlModifier},
		    std::pair{Qt::Key_Meta, Qt::MetaModifier},
		    std::pair{Qt::Key_Alt, Qt::AltModifier},
		};

		constexpr Qt::KeyboardModifier fromKey(int key) noexcept {
			auto const qtKey = static_cast<Qt::Key>(key);

			for (auto const& [modKey, modifier] : mods) {
				if (modKey == qtKey) return modifier;
			}

			return Qt::NoModifier;
		}

		void keyPressEvent(ShortcutDiscovery& disco, QKeyEvent* event) {
			auto const mod = fromKey(event->key());
			if (mod != Qt::NoModifier) {
				disco.modifiersPressed(mod);
			} else {
				disco.cancelDiscovery();
			}
		}

		void keyReleaseEvent(ShortcutDiscovery& disco, QKeyEvent* event) {
			auto const mod = fromKey(event->key());
			if (mod != Qt::NoModifier) {
				disco.modifiersReleased(mod);
			} else {
				disco.cancelDiscovery();
			}
		}
	}  // namespace

	ShortcutDiscovery::ShortcutDiscovery(ch::milliseconds delay) {
		timer_.setSingleShot(true);
		timer_.setInterval(delay.count());
		timer_.callOnTimeout(this, &ShortcutDiscovery::timedOut);
		qApp->installEventFilter(this);
	}

	bool ShortcutDiscovery::eventFilter(QObject* watched, QEvent* event) {
		switch (event->type()) {
			case QEvent::KeyPress:
				keyPressEvent(*this, static_cast<QKeyEvent*>(event));
				break;
			case QEvent::KeyRelease:
				keyReleaseEvent(*this, static_cast<QKeyEvent*>(event));
				break;
			case QEvent::MouseButtonPress:
			case QEvent::MouseButtonRelease:
			case QEvent::MouseButtonDblClick:
			case QEvent::Wheel:
				cancelDiscovery();
				break;
			default:
				break;
		}
		return QObject::eventFilter(watched, event);
	}

	void ShortcutDiscovery::modifiersPressed(Qt::KeyboardModifiers mods) {
		if (!setModifiers(modifiers_ | mods)) return;

		if (active_) {
			updateOverlay();
		} else if (!timer_.isActive()) {
			timer_.start();
		}  // else continue the timer
	}

	void ShortcutDiscovery::modifiersReleased(Qt::KeyboardModifiers mods) {
		if (!setModifiers(modifiers_ & ~mods)) return;

		if (!modifiers_) {
			cancelDiscovery();
		} else if (active_) {
			updateOverlay();
		}  // else continue the timer
	}

	void ShortcutDiscovery::cancelDiscovery() {
		timer_.stop();
		if (!setActive(false)) return;
		setModifiers(Qt::NoModifier);
		updateOverlay();
	}

	void ShortcutDiscovery::timedOut() {
		auto const active = modifiers_ != Qt::KeyboardModifiers{};
		if (active_ == active) return;
		active_ = active;
		updateOverlay();
	}

	bool ShortcutDiscovery::setModifiers(Qt::KeyboardModifiers value) {
		if (modifiers_ == value) return false;
		modifiers_ = value;
		return true;
	}

	bool ShortcutDiscovery::setActive(bool value) {
		if (active_ == value) return false;
		active_ = value;
		return true;
	}

	void ShortcutDiscovery::updateOverlay() { modifiersChanged(modifiers_); }
}  // namespace quick_dra::gui
