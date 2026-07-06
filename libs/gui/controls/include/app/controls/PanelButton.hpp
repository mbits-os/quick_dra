// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QKeySequence>
#include <QLayout>
#include <QList>
#include <QWidget>
#include <app/utils/DevicePixelScale.hpp>
#include <memory>

namespace quick_dra::gui {
	class PanelButtonPrivate;
	class PanelButton : public QObject {
		Q_OBJECT
		Q_DECLARE_PRIVATE(PanelButton)
		Q_DISABLE_COPY_MOVE(PanelButton)

	public:
		PanelButton();
		~PanelButton();

		void setSequences(QList<QKeySequence> const&);

		QWidget* widget() const;
		QLayout* layout() const;
		void clearItem();

		bool isClickable() const noexcept;
		bool isEnabled() const noexcept;
		bool isHovered() const noexcept;
		bool isActive() const noexcept;
		bool isFocused() const noexcept;

		QString const& toolTip() const noexcept;
		void setToolTip(QString const&);

	public slots:
		void setClickable(bool value) noexcept;
		void setEnabled(bool value) noexcept;
		void setHovered(bool value) noexcept;
		void setActive(bool value) noexcept;
		void setFocused(bool value) noexcept;
		void setPageFocused(bool value) noexcept;

	signals:
		void clicked();

	private:
		friend class PanelButtonGroup;
		friend class PanelButtonGroupPrivate;
		std::unique_ptr<PanelButtonPrivate> d_ptr{};
	};
}  // namespace quick_dra::gui
