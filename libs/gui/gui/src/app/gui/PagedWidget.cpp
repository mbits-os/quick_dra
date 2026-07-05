// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QApplication>
#include <app/gui/PageFocusEvent.hpp>
#include <app/gui/PagedWidget.hpp>

namespace quick_dra::gui {
	namespace {
		void broadcastPageFocusEvent(QWidget* parent, bool hasFocus) {
			auto const children = parent->findChildren<QWidget*>();
			for (auto const child : children) {
				qApp->notify(child, new PageFocusEvent{hasFocus});
			}
		}
	}  // namespace

	PagedWidget::PagedWidget(QWidget* parent) : QWidget{parent} {
		QObject::connect(this, &QWidget::windowTitleChanged, this, &PagedWidget::onTitleChange);
	}

	void PagedWidget::pageAdded(Globals* globals) {
		globals_ = globals;
		connectPage();
	}

	void PagedWidget::connectPage() {}
	void PagedWidget::beforePageFocus() {}
	void PagedWidget::pageFocus() { broadcastPageFocusEvent(this, true); }
	void PagedWidget::beforePageBlur() { broadcastPageFocusEvent(this, false); }
	void PagedWidget::pageBlur() {}
	void PagedWidget::pageRemoved() {}
	void PagedWidget::accept() { accepted(); }
	void PagedWidget::leavePage() { stack().navigateBack(); }
	void PagedWidget::setupPageUI() {
		stack().setFormDirty(formDirty_);
		stack().setFormValid(formValid_);
		onTitleChange(windowTitle());
	}
	bool PagedWidget::survivesReload() const { return false; }

	void PagedWidget::setFormDirty(bool value) {
		if (formDirty_ == value) {
			return;
		}

		formDirty_ = value;

		if (stack().page() != this) {
			return;
		}
		stack().setFormDirty(formDirty_);
	}

	void PagedWidget::setFormValid(bool value) {
		if (formValid_ == value) {
			return;
		}

		formValid_ = value;

		if (stack().page() != this) {
			return;
		}
		stack().setFormValid(formValid_);
	}

	void PagedWidget::onTitleChange(QString const& title) {
		if (!globals_ || !globals().hasStack() || stack().page() != this) {
			return;
		}
		stack().setHeaderTitle(title);
	}
}  // namespace quick_dra::gui
