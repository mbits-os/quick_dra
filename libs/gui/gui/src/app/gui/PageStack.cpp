// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <app/gui/PageHeader.hpp>
#include <app/gui/PageStack.hpp>
#include <app/gui/PagedWidget.hpp>

namespace quick_dra::gui {
	namespace {
		PageStack* currentStack{};

		void swapPages(PageStack* stack, PagedWidget* previous, PagedWidget* next, auto&& action) {
			if (previous) {
				previous->beforePageBlur();
			}
			next->beforePageFocus();

			action();

			if (previous) {
				previous->pageBlur();
			}
			next->setupPageUI();
			stack->updateNavigateBackButton();
			next->pageFocus();
		}
	}  // namespace

	PageStack::PageStack(PageHeader* globalHeader, QStackedWidget* parent)
	    : QObject{parent}, globalHeader_{globalHeader}, parent_{parent} {
		currentStack = this;
		setupUi();
	}

	PageStack* PageStack::current() { return currentStack; }

	void PageStack::setGlobals(Globals* globalsPtr) { globals_ = globalsPtr; }

	bool PageStack::formDirty() const noexcept { return globalHeader_->formDirty(); }
	bool PageStack::formValid() const noexcept { return globalHeader_->formValid(); }
	bool PageStack::topMost() const noexcept { return globalHeader_->topMost(); }

	void PageStack::setupUi() {
		QObject::connect(parent_, &QStackedWidget::currentChanged, this, &PageStack::currentChanged);
		QObject::connect(globalHeader_, &PageHeader::navigatingBack, this, &PageStack::navigateBack);
		QObject::connect(globalHeader_, &PageHeader::changesAccepted, this, &PageStack::formAccepted);
	}

	PagedWidget* PageStack::page() { return qobject_cast<PagedWidget*>(parent_->currentWidget()); }

	void PageStack::pushPage(PagedWidget* newPage) {
		if (newPage->parent() == nullptr) newPage->setParent(parent_);
		newPage->pageAdded(globals_);

		swapPages(this, page(), newPage, [parent = parent_, newPage] {
			auto const index = parent->addWidget(newPage);
			parent->setCurrentIndex(index);
		});
	}

	void PageStack::navigateBack() {
		if (parent_->count() < 2) {
			// either home page not yet pushed, or only home page present;
			return;
		}

		auto const index = parent_->currentIndex() - 1;

		auto const currentPage = page();
		auto const nextPage = qobject_cast<PagedWidget*>(parent_->widget(index));

		swapPages(this, currentPage, nextPage, [parent = parent_, currentPage] { parent->removeWidget(currentPage); });

		currentPage->pageRemoved();
		currentPage->deleteLater();
	}

	void PageStack::navigateHomeForReload() {
		while (parent_->count() > 1) {
			auto const currentPage = page();
			if (currentPage->survivesReload()) {
				break;
			}

			navigateBack();
		}
	}

	void PageStack::updateNavigateBackButton() { globalHeader_->setTopMost(parent_->currentIndex() == 0); }
	void PageStack::setFormDirty(bool value) { globalHeader_->setFormDirty(value); }
	void PageStack::setFormValid(bool value) { globalHeader_->setFormValid(value); }
	void PageStack::formAccepted() { page()->accept(); }

	void PageStack::setHeaderTitle(QString const& title) {
		globalHeader_->setTitle(title);
		titleChanged();
	}

	void PageStack::currentChanged(int) { pageChanged(); }
}  // namespace quick_dra::gui
