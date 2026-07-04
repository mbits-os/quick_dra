// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QWidget>
#include <app/utils/DevicePixelScale.hpp>
#include <memory>
#include <optional>

namespace quick_dra::gui {
	struct PanelInfo {
		QString label{};
		QString details{};
		QString value{};
		QIcon rightIcon{};
		std::optional<bool> isClickable{true};
		std::optional<bool> isEnabled{};
	};

	class PanelPrivate;

	class PanelPrivate;
	class Panel : public QWidget {
		Q_OBJECT
		Q_DECLARE_PRIVATE(Panel)
		Q_DISABLE_COPY_MOVE(Panel)

	public:
		Panel(QWidget* parent = nullptr);
		~Panel();

		[[deprecated("Use setInfo(PanelInfo const&)")]] void setInfo(QString const& label,
		                                                             QString const& details,
		                                                             QString const& value,
		                                                             QIcon const& rightIcon);
		void setInfo(PanelInfo const&);
		void setTitle(QString const& text) noexcept;
		void setValue(QString const& text) noexcept;
		void setDetails(QString const& text) noexcept;
		void setIcon(QIcon const& icon) noexcept;

		bool hasHeightForWidth() const override;
		int heightForWidth(int) const override;
		QSize sizeHint() const override;
		QSize minimumSizeHint() const override;

		bool event(QEvent*) override;
		void resizeEvent(QResizeEvent*) override;
		void paintEvent(QPaintEvent*) override;

	private:
		std::unique_ptr<PanelPrivate> d_ptr;
	};
}  // namespace quick_dra::gui
