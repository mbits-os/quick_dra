// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QKeySequence>
#include <QList>
#include <QTimer>
#include <QWidget>
#include <Qt>
#include <chrono>
#include <concepts>
#include <deque>
#include <vector>

using namespace std::literals;
namespace ch = std::chrono;

QT_FORWARD_DECLARE_CLASS(QToolButton)

namespace quick_dra::gui {
	class ToolTipLabel;

	struct HolderPosition {
		QRect geometry;
		QWidget const* rectReference;
	};

	template <typename T>
	struct HolderSupport {};

	namespace utility {
		template <typename API, typename T>
		concept SupportsHolder = requires(T* obj, T const* cobj) {
			{ API::isEnabled(cobj) } -> std::same_as<bool>;
			{ API::keys(obj) } -> std::same_as<QList<QKeySequence>>;
			{ API::getPosition(obj) } -> std::same_as<HolderPosition>;
		};

		template <typename T>
		concept KeySequencesHolder = SupportsHolder<HolderSupport<T>, T>;
	}  // namespace utility

	template <>
	struct HolderSupport<QWidget> {
		static bool isEnabled(QWidget const* holder) noexcept;
		static QList<QKeySequence> keys(QWidget*) { return {}; }
		static HolderPosition getPosition(QWidget const* obj) noexcept;
	};

	template <>
	struct HolderSupport<QToolButton> : HolderSupport<QWidget> {
		static QList<QKeySequence> keys(QToolButton const* holder);
	};

	struct ShortcutHolderInfo {
		QObject* holder;
		QList<QKeySequence> keys;
		bool (*isEnabled)(QObject const*) noexcept = nullptr;
		HolderPosition (*getPosition)(QObject const*) = nullptr;

		auto operator<=>(ShortcutHolderInfo const&) const noexcept = default;

		template <utility::KeySequencesHolder Holder>
		static ShortcutHolderInfo from(Holder* holder) {
			using Impl = ImplFor<Holder>;

			return ShortcutHolderInfo{
			    .holder = holder,
			    .keys = Impl::keys(holder),
			    .isEnabled = Impl::isEnabled,
			    .getPosition = Impl::getPosition,
			};
		}

		template <utility::KeySequencesHolder Holder>
		struct ImplFor {
			using Support = HolderSupport<Holder>;
			static_assert(utility::SupportsHolder<Support, Holder>);

			static QList<QKeySequence> keys(Holder* holder) { return Support::keys(holder); }
			static bool isEnabled(QObject const* holder) noexcept {
				return Support::isEnabled(static_cast<Holder const*>(holder));
			}
			static HolderPosition getPosition(QObject const* holder) {
				return Support::getPosition(static_cast<Holder const*>(holder));
			}
		};
	};

	class ShortcutDiscovery : public QObject {
		Q_OBJECT

		struct PrivateTag {};

	public:
		struct LabelInfo {
			QWidget* parent = nullptr;
			ToolTipLabel* toolTip = nullptr;
			QPoint origin{};
			QString text{};

			bool operator==(LabelInfo const& rhs) const noexcept {
				return parent == rhs.parent && origin == rhs.origin && text == rhs.text;
			}
		};

		struct Editor {
			static Editor* current;

			Editor(Editor const&) = delete;
			Editor() = delete;

			explicit Editor(ShortcutDiscovery*, PrivateTag = {});
			Editor(Editor&&);
			~Editor();

			template <utility::KeySequencesHolder Holder>
			Editor& addHolder(Holder* holder) {
				addHolderFromInfo(ShortcutHolderInfo::from(holder));
				return *this;
			}
			Editor& removeHolder(QObject* holder);

		private:
			void addHolderFromInfo(ShortcutHolderInfo const& info);

			Editor* previous_{};
			ShortcutDiscovery* parent_{};
			std::deque<ShortcutHolderInfo> holders_{};
		};

		ShortcutDiscovery(ch::milliseconds delay = 1s);

		bool eventFilter(QObject* watched, QEvent* event) override;

		bool isActive() const noexcept { return active_; }

		Editor beginHolderUpdate();
		std::deque<ShortcutHolderInfo> const& holders() const noexcept { return holders_; }
		std::vector<LabelInfo> const& labels() const noexcept { return labels_; }

	public slots:
		void modifiersPressed(Qt::KeyboardModifiers);
		void modifiersReleased(Qt::KeyboardModifiers);
		void cancelDiscovery();

	private slots:
		void timedOut();

	signals:
		void modifiersChanged(Qt::KeyboardModifiers);
		void holdersChanged();
		void labelsChanged();

	private:
		enum class ModsState {
			Stable = false,
			Changed = true,
		};

		void endHolderUpdate(std::deque<ShortcutHolderInfo>&& holders);
		bool setModifiers(Qt::KeyboardModifiers value);
		bool setActive(bool value);
		void updateOverlay(ModsState modsChanged);
		std::vector<LabelInfo> filterShortcuts();

		bool active_{false};
		QTimer timer_{};
		Qt::KeyboardModifiers modifiers_{};
		std::deque<ShortcutHolderInfo> holders_{};
		std::vector<LabelInfo> labels_{};
	};
}  // namespace quick_dra::gui
