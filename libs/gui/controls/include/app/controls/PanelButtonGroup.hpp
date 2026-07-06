// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QEvent>
#include <QVBoxLayout>
#include <QWidget>
#include <app/utils/DevicePixelScale.hpp>
#include <app/utils/empty_callback.hpp>
#include <memory>
#include <utility>

namespace quick_dra::gui {
	class PanelButton;
	class PanelButtonGroupPrivate;

	class PanelButtonGroup : public QWidget {
		Q_OBJECT
		Q_DECLARE_PRIVATE(PanelButtonGroup)
		Q_DISABLE_COPY_MOVE(PanelButtonGroup)

	public:
		PanelButtonGroup(QWidget* parent = nullptr);
		~PanelButtonGroup();

		DevicePixelScale const& scale() const noexcept;

		int count() const;
		PanelButton* itemAt(int) const;

		PanelButton* addButton(QString const&, bool bold = false);
		PanelButton* addWidget(QWidget*);
		PanelButton* addLayout(QLayout*);

		template <typename Wgt, std::invocable<Wgt&> Callback = EmptyCallback<Wgt>>
		PanelButton* createWidget(QAnyStringView objectName, Callback&& cb = {}) {
			auto object = std::make_unique<Wgt>(this);
			object->setObjectName(objectName);
			cb(*object.get());
			auto result = addWidget(object.get());
			object.release();
			return result;
		}

		template <typename Wgt, std::invocable<Wgt&> Callback = EmptyCallback<Wgt>>
		PanelButton* createWidget(Callback&& cb = {}) {
			return this->createWidget<Wgt, Callback>("", std::forward<Callback>(cb));
		}

		struct CreatePanelOptions {
			QString label{};
			QString details{};
			QString value{};
			QString toolTip{};
			QIcon rightIcon{};
			std::optional<bool> isClickable{true};
			std::optional<bool> isEnabled{};
			QList<QKeySequence> sequences{};
		};

		PanelButton* createPanel(CreatePanelOptions const&, QAnyStringView objectName = {});

		PanelButton* takeLast();
		void clearAll();

		bool event(QEvent* event) override;
		void paintEvent(QPaintEvent* event) override;
		void enterEvent(QEnterEvent* event) override;
		void leaveEvent(QEvent* event) override;
		void mouseMoveEvent(QMouseEvent* event) override;
		void mousePressEvent(QMouseEvent* event) override;
		void mouseReleaseEvent(QMouseEvent* event) override;
		void keyPressEvent(QKeyEvent* event) override;
		void keyReleaseEvent(QKeyEvent* event) override;
		void focusInEvent(QFocusEvent* event) override;
		void focusOutEvent(QFocusEvent* event) override;

		bool focusNextPrevChild(bool next) override;

	private:
		std::unique_ptr<PanelButtonGroupPrivate> d_ptr{};
	};
}  // namespace quick_dra::gui
