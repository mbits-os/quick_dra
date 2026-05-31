// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QLabel>
#include <QToolBar>
#include <QWidget>

namespace quick_dra::gui {
	class HeaderToolbar : public QToolBar {
		Q_OBJECT
	public:
		using QToolBar::QToolBar;

		void resizeEvent(QResizeEvent* event) override;

	signals:
		void widthChanged(int width);

	private:
		int width_{-1};
	};

	class HeaderSpacer : public QWidget {
		Q_OBJECT
	public:
		using QWidget::QWidget;

		QSize sizeHint() const override;
		QSize minimumSizeHint() const override;

		int hintWidth() const noexcept { return hintWidth_; }

	public slots:
		void setHintWidth(int);

	private:
		int hintWidth_{0};
	};

	class HeaderShadow : public QWidget {
		Q_OBJECT
	public:
		using QWidget::QWidget;

		QSize sizeHint() const override;
		QSize minimumSizeHint() const override;
		void paintEvent(QPaintEvent*) override;

		int shadowHeight() const noexcept { return shadowHeight_; }
		float shadowForce() const noexcept { return shadowForce_; }

	public slots:
		void setShadowHeight(int);
		void setShadowForce(float);
		void targetMoved(QRect const&);

	private:
		int shadowHeight_{0};
		float shadowForce_{1.f};
	};

	class PageHeader : public QWidget {
		Q_OBJECT

	public:
		explicit PageHeader(QWidget* parent);

		HeaderToolbar* toolBar() const noexcept { return toolBar_; }
		bool formDirty() const noexcept { return formDirty_; }
		bool topMost() const noexcept { return topMost_; }

		void paintEvent(QPaintEvent*) override;
		void resizeEvent(QResizeEvent*) override;

	public slots:
		void leavePage();
		void acceptChanges();

		// PAGE STACK API
		void setFormDirty(bool);
		void setFormValid(bool);
		void setTitle(QString const&);
		void setTopMost(bool);

	signals:
		void navigatingBack();
		void changesAccepted();
		void showActionBack(bool);
		void formDirtyChanged(bool);
		void formValidChanged(bool);
		void moved(QRect const&);

	private:
		void setupUI();

		bool formDirty_{false};
		bool formValid_{true};
		bool topMost_{false};
		HeaderToolbar* toolBar_{};
		QLabel* titleLabel_{};
	};
}  // namespace quick_dra::gui
