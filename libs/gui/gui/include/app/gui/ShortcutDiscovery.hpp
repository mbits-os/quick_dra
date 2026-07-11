// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QTimer>
#include <Qt>
#include <chrono>

using namespace std::literals;
namespace ch = std::chrono;

namespace quick_dra::gui {
	class ShortcutDiscovery : public QObject {
		Q_OBJECT

	public:
		ShortcutDiscovery(ch::milliseconds delay = 1s);

		bool eventFilter(QObject* watched, QEvent* event) override;

		bool isActive() const noexcept { return active_; }

	public slots:
		void modifiersPressed(Qt::KeyboardModifiers);
		void modifiersReleased(Qt::KeyboardModifiers);
		void cancelDiscovery();

	private slots:
		void timedOut();

	signals:
		void modifiersChanged(Qt::KeyboardModifiers);

	private:
		bool setModifiers(Qt::KeyboardModifiers value);
		bool setActive(bool value);
		void updateOverlay();

		bool active_{false};
		QTimer timer_{};
		Qt::KeyboardModifiers modifiers_{};
	};
}  // namespace quick_dra::gui
