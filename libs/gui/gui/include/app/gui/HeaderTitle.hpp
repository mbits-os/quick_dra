// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QWidget>
#include <app/gui/types.hpp>
#include <memory>

namespace quick_dra::gui {
	class HeaderTitlePrivate;
	class HeaderTitle : public QWidget {
		Q_OBJECT
		Q_PROPERTY(qreal animationProgress READ animationProgress WRITE setAnimationProgress)

	public:
		HeaderTitle(QWidget* parent = nullptr);
		~HeaderTitle();

		qreal animationProgress() const;

		QSize sizeHint() const override;
		QSize minimumSizeHint() const override;
		void paintEvent(QPaintEvent*) override;

		void beforeAnimation(PageChangeDirection);

	public slots:
		void setText(const QString&);
		void setAlignment(Qt::Alignment);
		void setAnimationProgress(qreal);

	signals:
		void animationDirectionChange(int);

	private:
		Q_DISABLE_COPY(HeaderTitle)
		Q_DECLARE_PRIVATE(HeaderTitle)

		std::unique_ptr<HeaderTitlePrivate> q_ptr{};
	};
}  // namespace quick_dra::gui
