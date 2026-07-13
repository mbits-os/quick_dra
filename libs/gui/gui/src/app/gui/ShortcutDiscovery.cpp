// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QApplication>
#include <QDebug>
#include <QKeyEvent>
#include <QToolButton>
#include <QToolTip>
#include <algorithm>
#include <app/gui/Globals.hpp>
#include <app/gui/ShortcutDiscovery.hpp>
#include <array>
#include <utility>
#include <vector>

namespace quick_dra::gui {
	namespace {
		static constexpr auto modMap = std::array{
		    std::pair{Qt::Key_Shift, Qt::ShiftModifier},
		    std::pair{Qt::Key_Control, Qt::ControlModifier},
		    std::pair{Qt::Key_Meta, Qt::MetaModifier},
		    std::pair{Qt::Key_Alt, Qt::AltModifier},
		};

		constexpr Qt::KeyboardModifier fromKey(int key) noexcept {
			auto const qtKey = static_cast<Qt::Key>(key);

			for (auto const& [modKey, modifier] : modMap) {
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

	bool HolderSupport<QWidget>::isEnabled(QWidget const* widget) noexcept {
		return widget->isEnabled() && widget->isVisible();
	}

	HolderPosition HolderSupport<QWidget>::getPosition(QWidget const* obj) noexcept {
		return {
		    .geometry = obj->rect(),
		    .rectReference = obj,
		};
	}

	QList<QKeySequence> HolderSupport<QToolButton>::keys(QToolButton const* holder) {
		auto const action = holder->defaultAction();
		if (!action) {
			return {};
		}
		return action->shortcuts();
	}

	ShortcutDiscovery::Editor* ShortcutDiscovery::Editor::current = nullptr;

	ShortcutDiscovery::Editor::Editor(ShortcutDiscovery* parent, PrivateTag)
	    : previous_{current}, parent_{parent}, holders_{parent->holders_} {
		current = this;
	}

	ShortcutDiscovery::Editor::Editor(Editor&& rhs)
	    : previous_{rhs.previous_}, parent_{rhs.parent_}, holders_{std::move(rhs.holders_)} {
		current = this;
		rhs.parent_ = nullptr;
	}

	ShortcutDiscovery::Editor::~Editor() {
		if (parent_) {
			parent_->endHolderUpdate(std::move(holders_));
			current = previous_;
		}
	}

	void ShortcutDiscovery::Editor::addHolderFromInfo(ShortcutHolderInfo const& info) {
#define NONNULL(X) !info.X
		if (NONNULL(holder) || info.keys.isEmpty() || NONNULL(getPosition) || NONNULL(isEnabled)) {
			return;
		}
		holders_.emplace_back(info);
	}

	ShortcutDiscovery::Editor& ShortcutDiscovery::Editor::removeHolder(QObject* holder) {
		auto it = std::find_if(holders_.begin(), holders_.end(),
		                       [holder](auto const& item) { return item.holder == holder; });
		if (it != holders_.end()) {
			holders_.erase(it);
		}

		return *this;
	}

	ShortcutDiscovery::ShortcutDiscovery(ch::milliseconds delay) {
		timer_.setSingleShot(true);
		timer_.setInterval(static_cast<int>(delay.count()));
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
			updateOverlay(ModsState::Changed);
		} else if (!timer_.isActive()) {
			timer_.start();
		}  // else continue the timer
	}

	void ShortcutDiscovery::modifiersReleased(Qt::KeyboardModifiers mods) {
		if (!setModifiers(modifiers_ & ~mods)) return;

		if (!modifiers_) {
			cancelDiscovery();
		} else if (active_) {
			updateOverlay(ModsState::Changed);
		}  // else continue the timer
	}

	void ShortcutDiscovery::cancelDiscovery() {
		timer_.stop();
		if (!setActive(false)) return;
		setModifiers(Qt::NoModifier);
		updateOverlay(ModsState::Changed);
	}

	void ShortcutDiscovery::timedOut() {
		auto const active = modifiers_ != Qt::KeyboardModifiers{};
		if (active_ == active) return;
		active_ = active;
		updateOverlay(ModsState::Changed);
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

	ShortcutDiscovery::Editor ShortcutDiscovery::beginHolderUpdate() { return Editor{this}; }

	void ShortcutDiscovery::endHolderUpdate(std::deque<ShortcutHolderInfo>&& holders) {
		if (holders_ == holders) {
			return;
		}

		holders_ = std::move(holders);
		holdersChanged();

		if (active_) {
			updateOverlay(ModsState::Stable);
		}
	}

	void ShortcutDiscovery::updateOverlay(ModsState modsChanged) {
		if (modsChanged == ModsState::Changed) {
			modifiersChanged(modifiers_);
		}
		auto labels = filterShortcuts();
		if (labels == labels_) {
			return;
		}

		for (auto const& label : labels_) {
			if (label.toolTip) label.toolTip->deleteLater();  // GCOV_EXCL_LINE (until #106)
		}
		labels_ = std::move(labels);
		for (auto const& label : labels_) {
			// TODO: mbits-os/quick_dra#106 Show matching shortcuts, when Control is being held for one seconds
		}

		labelsChanged();
	}

	std::vector<ShortcutDiscovery::LabelInfo> ShortcutDiscovery::filterShortcuts() {
		if (modifiers_ == Qt::NoModifier) {
			return {};
		}

		size_t count = 0;

		for (auto const& holder : holders_) {
			if (!holder.isEnabled(holder.holder)) continue;
			for (auto const& key : holder.keys) {
				if (key.count() != 1) continue;
				auto const& mod = key[0];
				if ((mod.keyboardModifiers() & modifiers_) == modifiers_) {
					++count;
				}
			}
		}

		std::vector<LabelInfo> labels{};
		labels.reserve(count);

		QFontMetrics fm{QToolTip::font()};
		int lineHeight{fm.ascent() + fm.descent() + fm.leading()};
		int em{fm.boundingRect("m").width()};

		int vOffset = -lineHeight * 2 / 3;

		for (auto const& holder : holders_) {
			if (!holder.isEnabled(holder.holder)) continue;
			int hOffset = em;
			for (auto const& key : holder.keys) {
				if (key.count() != 1) continue;
				auto const& mod = key[0];
				if ((mod.keyboardModifiers() & modifiers_) == modifiers_) {
					auto const pos = holder.getPosition(holder.holder);
					auto const topLevelParent = pos.rectReference->topLevelWidget();

					auto const position =
					    pos.rectReference->mapTo(topLevelParent, pos.geometry.bottomLeft()) + QPoint{hOffset, vOffset};
					auto label = QKeySequence{mod.key() | (mod.keyboardModifiers() & ~modifiers_)}.toString();
					hOffset += 2 * em + fm.boundingRect(label).width();

					labels.emplace_back(
					    LabelInfo{.parent = topLevelParent, .origin = position, .text = std::move(label)});
				}
			}
		}

		return labels;
	}
}  // namespace quick_dra::gui
