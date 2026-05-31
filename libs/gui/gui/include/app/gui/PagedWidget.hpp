// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QWidget>
#include <app/gui/Globals.hpp>
#include <app/gui/PageStack.hpp>

namespace quick_dra::gui {
	class PagedWidget : public QWidget {
		Q_OBJECT

	public:
		PagedWidget(QWidget* parent = nullptr);

		virtual void pageAdded(Globals*);
		virtual void connectPage();
		virtual void beforePageFocus();
		virtual void pageFocus();
		virtual void beforePageBlur();
		virtual void pageBlur();
		virtual void pageRemoved();
		virtual void accept();
		virtual void setupPageUI();
		virtual bool survivesReload() const;

		bool formDirty() const noexcept { return formDirty_; }
		bool formValid() const noexcept { return formValid_; }

	public slots:
		void leavePage();
		void acceptChanges();
		void setFormDirty(bool);
		void setFormValid(bool);

	private slots:
		void onTitleChange(QString const& title);

	signals:
		void accepted();

	protected:
		Globals& globals() const noexcept { return *globals_; }
		PageStack& stack() const noexcept { return globals().stack(); }

		template <std::derived_from<PagedWidget> T>
		T* push() {
			return stack().push<T>();
		}

	private:
		Globals* globals_{};
		bool formDirty_{false};
		bool formValid_{true};
	};
}  // namespace quick_dra::gui
