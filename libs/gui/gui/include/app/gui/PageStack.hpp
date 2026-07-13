// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QObject>
#include <QStackedWidget>
#include <app/gui/ShortcutDiscovery.hpp>
#include <app/gui/types.hpp>
#include <memory>
#include <utility>

namespace quick_dra::gui {
	class PagedWidget;
	class PageHeader;
	class Globals;

	class PageStack : public QObject {
		Q_OBJECT

	public:
		PageStack(PageHeader* globalHeader, QStackedWidget* parent = nullptr);

		static PageStack* current();

		void setGlobals(Globals* globals);
		Globals& globals() const noexcept { return *globals_; }
		ShortcutDiscovery& discovery() noexcept { return discovery_; }
		bool formDirty() const noexcept;
		bool formValid() const noexcept;
		bool topMost() const noexcept;

		PagedWidget* page();

		template <std::derived_from<PagedWidget> T, typename... Args>
		T* push(Args&&... args)
		    requires requires(Args&&... args) { new T{std::forward<Args>(args)...}; }
		{
			auto ref = std::make_unique<T>(std::forward<Args>(args)...);
			pushPage(ref.get());
			return ref.release();
		}

	public slots:
		void pushPage(PagedWidget* page);
		void navigateBack();
		void navigateHomeForReload();

		// HEADER SLOTS:
		void setHeaderTitle(QString const&);
		void updateNavigateBackButton();
		void setFormDirty(bool);
		void setFormValid(bool);
		void formAccepted();
		void animatePageChange(PageChangeDirection);

	private slots:
		void currentChanged(int);

	signals:
		void pageChanged();
		void titleChanged();

	private:
		void setupUi();

		Globals* globals_{};

		PageHeader* globalHeader_{};
		QStackedWidget* parent_{};
		ShortcutDiscovery discovery_{};
	};
}  // namespace quick_dra::gui
