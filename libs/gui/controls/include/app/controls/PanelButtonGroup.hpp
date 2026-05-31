// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QWidget>
#include <concepts>
#include <memory>
#include <utility>

namespace quick_dra::gui {
	class PanelButtonPrivate;
	class PanelButtonGroupPrivate;

	class PanelButton : public QObject {
		Q_OBJECT
		Q_DECLARE_PRIVATE(PanelButton)
		Q_DISABLE_COPY_MOVE(PanelButton)

	public:
		PanelButton();
		~PanelButton();

		QWidget* widget() const;
		QLayout* layout() const;
		void clearItem();

		bool isClickable() const noexcept;
		bool isEnabled() const noexcept;
		bool isHovered() const noexcept;
		bool isActive() const noexcept;

	public slots:
		void setClickable(bool value) noexcept;
		void setEnabled(bool value) noexcept;
		void setHovered(bool value) noexcept;
		void setActive(bool value) noexcept;

	signals:
		void clicked();

	private:
		friend class PanelButtonGroup;
		friend class PanelButtonGroupPrivate;
		std::unique_ptr<PanelButtonPrivate> d_ptr{};
	};

	class PanelButtonGroup : public QWidget {
		Q_OBJECT
		Q_DECLARE_PRIVATE(PanelButtonGroup)
		Q_DISABLE_COPY_MOVE(PanelButtonGroup)

	public:
		PanelButtonGroup(QWidget* parent = nullptr);
		~PanelButtonGroup();

		PanelButton* addButton(QString const&, bool bold = false);
		PanelButton* addWidget(QWidget*);
		PanelButton* addLayout(QLayout*);

		template <typename Wgt, std::invocable<Wgt&> Callback = decltype([](auto const&) {})>
		PanelButton* createWidget(QAnyStringView objectName, Callback&& cb = {}) {
			auto object = std::make_unique<Wgt>(this);
			object->setObjectName(objectName);
			cb(*object.get());
			auto result = addWidget(object.get());
			object.release();
			return result;
		}

		template <typename Wgt, std::invocable<Wgt&> Callback = decltype([](auto const&) {})>
		PanelButton* createWidget(Callback&& cb = {}) {
			return this->createWidget<Wgt, Callback>("", std::forward<Callback>(cb));
		}

		PanelButton* takeLast();
		void clearAll();

		void paintEvent(QPaintEvent* event) override;
		void enterEvent(QEnterEvent* event) override;
		void leaveEvent(QEvent* event) override;
		void mouseMoveEvent(QMouseEvent* event) override;
		void mousePressEvent(QMouseEvent* event) override;
		void mouseReleaseEvent(QMouseEvent* event) override;

	private:
		std::unique_ptr<PanelButtonGroupPrivate> d_ptr{};
	};

	class Panel : public QWidget {
		Q_OBJECT
		Q_DISABLE_COPY_MOVE(Panel)

	public:
		Panel(QWidget* parent = nullptr);

		void setInfo(QString const& label, QString const& details, QString const& value, QIcon const& rightIcon);

		QLabel* value() const noexcept { return value_; }

	private:
		QHBoxLayout* root_{};
		QLabel* value_{};
	};
}  // namespace quick_dra::gui
