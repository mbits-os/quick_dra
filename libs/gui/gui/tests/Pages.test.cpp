// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <gtest/gtest.h>
#include <QAction>
#include <QMouseEvent>
#include <QToolBar>
#include "TestApp.hpp"

namespace quick_dra::gui::testing {
#define ACTION_X(X)      \
	X(set_title)         \
	X(connect_page)      \
	X(before_page_focus) \
	X(page_focus)        \
	X(before_page_blur)  \
	X(page_blur)         \
	X(page_removed)      \
	X(accept)            \
	X(leave_page)        \
	X(accept_changes)    \
	X(setup_page_ui)
	enum class action {
#define LIST(N) N,
		ACTION_X(LIST)
#undef LIST
	};

	std::ostream& operator<<(std::ostream& out, action item) {
		switch (item) {
#define CASE(N)     \
	case action::N: \
		return out << "actions::" #N;
			ACTION_X(CASE)
#undef CASE
		};
		return out << "action{" << std::to_underlying(item) << '}';
	}

	struct action_item {
		action kind{};
		std::string argument{};
		auto operator<=>(action_item const&) const noexcept = default;

		friend std::ostream& operator<<(std::ostream& out, action_item const& item) {
			return out << "\n  " << item.kind << "(\"" << item.argument << "\"sv)";
		}
	};

	struct actions : QObject {
		actions* prev{};
		PageStack* stack{};
		std::vector<action_item> history{};
		bool pausing{false};

		static thread_local actions* current;

		actions() : prev{current} { current = this; }
		~actions() { current = prev; }

		void pause() { pausing = true; }
		void resume() { pausing = false; }

		void clickBack() {
			auto header = stack->parent()->findChild<QWidget*>("PageHeader");
			auto action = header->findChild<QAction*>("actionBack");
			action->triggered();
		}
		void clickAccept() {
			auto header = stack->parent()->findChild<QWidget*>("PageHeader");
			auto action = header->findChild<QAction*>("actionAccept");
			action->triggered();
		}

#define CREATE(N)                                                                                           \
	static action_item N(std::string_view title) { return {.kind = action::N, .argument = as_str(title)}; } \
	static action_item N(QString const& title) { return N(as_sv(title.toUtf8())); }
		ACTION_X(CREATE)
#undef CREATE

		static void add(action_item const& item) {
			if (!current->pausing) current->history.push_back(item);
		}

	public slots:
		void title_changed() {
			auto const page = stack->page();
			auto const title = page ? page->windowTitle() : QString{};
			if (!pausing) history.push_back(set_title(title));
		}
	};

	thread_local actions* actions::current = nullptr;

	class PagedWidgetMock : public PagedWidget {
	public:
		PagedWidgetMock(QString const& title) { setWindowTitle(title); }

		std::optional<bool> overridenReload{};

		void connectPage() override {
			actions::add(actions::connect_page(windowTitle()));
			PagedWidget::connectPage();
		}
		void beforePageFocus() override {
			actions::add(actions::before_page_focus(windowTitle()));
			PagedWidget::beforePageFocus();
		}
		void pageFocus() override {
			actions::add(actions::page_focus(windowTitle()));
			PagedWidget::pageFocus();
		}
		void beforePageBlur() override {
			actions::add(actions::before_page_blur(windowTitle()));
			PagedWidget::beforePageBlur();
		}
		void pageBlur() override {
			actions::add(actions::page_blur(windowTitle()));
			PagedWidget::pageBlur();
		}
		void pageRemoved() override {
			actions::add(actions::page_removed(windowTitle()));
			PagedWidget::pageRemoved();
		}
		void accept() override {
			actions::add(actions::accept(windowTitle()));
			PagedWidget::accept();
		}
		void setupPageUI() override {
			actions::add(actions::setup_page_ui(windowTitle()));
			PagedWidget::setupPageUI();
		}
		bool survivesReload() const {
			if (overridenReload) return *overridenReload;
			return PagedWidget::survivesReload();
		}

		auto subpage(QString const& title) { return push<PagedWidgetMock>(title); }
	};

	struct PagesEnv : Env {
		actions tape{};

		bool formDirty() const noexcept { return stack->formDirty(); }
		bool formValid() const noexcept { return stack->formValid(); }
		bool topMost() const noexcept { return stack->topMost(); }

		PagesEnv() {
			tape.stack = stack;
			QObject::connect(stack, &PageStack::titleChanged, &tape, &actions::title_changed);
		}
	};

#define SWAP_PAGES(TITLE_OLD, TITLE_NEW)            \
	actions::before_page_blur(TITLE_OLD),      /**/ \
	    actions::before_page_focus(TITLE_NEW), /**/ \
	    actions::page_blur(TITLE_OLD),         /**/ \
	    actions::setup_page_ui(TITLE_NEW),     /**/ \
	    actions::set_title(TITLE_NEW),         /**/ \
	    actions::page_focus(TITLE_NEW)         /**/

	TEST(Pages, navigation) {
		platform::config_data_dir(form_data_config);
		PagesEnv env{};

		{
			auto homePage = env.stack->push<PagedWidgetMock>("Home Page");
			homePage->setWindowTitle("Home");
			EXPECT_FALSE(env.formDirty());
			EXPECT_TRUE(env.formValid());
			EXPECT_TRUE(env.topMost());

			auto subPage = homePage->subpage("Page #1");
			EXPECT_FALSE(env.formDirty());
			EXPECT_TRUE(env.formValid());
			EXPECT_FALSE(env.topMost());

			homePage->setWindowTitle("Home Page");
			subPage->setFormDirty(true);
			EXPECT_TRUE(env.formDirty());
			subPage->setFormDirty(false);
			EXPECT_FALSE(env.formDirty());

			subPage->subpage("Page #2");
			subPage->setFormDirty(true);
			EXPECT_FALSE(env.formDirty());
			subPage->setFormDirty(true);
			EXPECT_FALSE(env.formDirty());

			env.tape.clickBack();
			env.tape.clickAccept();
			subPage->leavePage();
			homePage->leavePage();  // should be no-op
		}

		std::vector<action_item> expected_history{
		    // push("Home Page")
		    actions::connect_page("Home Page"sv),
		    actions::before_page_focus("Home Page"sv),
		    actions::setup_page_ui("Home Page"sv),
		    actions::set_title("Home Page"sv),
		    actions::page_focus("Home Page"sv),
		    // setWindowTitle
		    actions::set_title("Home"sv),
		    // push("Page #1")
		    actions::connect_page("Page #1"sv),
		    SWAP_PAGES("Home"sv, "Page #1"sv),
		    // push("Page #2")
		    actions::connect_page("Page #2"sv),
		    SWAP_PAGES("Page #1"sv, "Page #2"sv),
		    // click back
		    SWAP_PAGES("Page #2"sv, "Page #1"sv),
		    actions::page_removed("Page #2"sv),
		    // clickAccept
		    actions::accept("Page #1"sv),
		    // leavePage
		    SWAP_PAGES("Page #1"sv, "Home Page"sv),
		    actions::page_removed("Page #1"sv),
		};

		EXPECT_EQ(Globals::current(), &env.globals);
		EXPECT_EQ(&env.stack->globals(), &env.globals);
		EXPECT_EQ(PageStack::current(), env.stack);
		EXPECT_EQ(env.tape.history, expected_history);
	}

	TEST(Pages, reload) {
		PagesEnv env{};

		{
			env.tape.pause();
			auto homePage = env.stack->push<PagedWidgetMock>("Home Page");
			auto page1 = homePage->subpage("Page #1");
			auto page2 = page1->subpage("Page #2");
			page2->subpage("Page #3")->subpage("Page #4");

			homePage->overridenReload = true;
			page2->overridenReload = true;

			env.tape.resume();
			env.stack->navigateHomeForReload();
		}

		std::vector<action_item> expected_history{
		    SWAP_PAGES("Page #4"sv, "Page #2"sv),
		    actions::page_removed("Page #4"sv),
		    actions::page_removed("Page #3"sv),
		};
		EXPECT_EQ(env.tape.history, expected_history);
	}

	TEST(Pages, dirty) {
		PagesEnv env{};

		auto homePage = env.stack->push<PagedWidgetMock>("Home Page");
		EXPECT_FALSE(env.formDirty());
		EXPECT_TRUE(env.formValid());
		EXPECT_TRUE(env.topMost());

		auto subPage = homePage->subpage("Page #1");
		EXPECT_FALSE(env.formDirty());
		EXPECT_TRUE(env.formValid());
		EXPECT_FALSE(env.topMost());

		subPage->setFormDirty(true);
		EXPECT_TRUE(env.formDirty());
		subPage->setFormDirty(false);
		EXPECT_FALSE(env.formDirty());
		subPage->setFormDirty(true);
		EXPECT_TRUE(env.formDirty());

		subPage->subpage("Page #2");
		EXPECT_FALSE(env.formDirty());
		subPage->setFormDirty(true);
		EXPECT_FALSE(env.formDirty());
		subPage->setFormDirty(true);
		EXPECT_FALSE(env.formDirty());
	}

	TEST(Pages, valid) {
		PagesEnv env{};

		auto homePage = env.stack->push<PagedWidgetMock>("Home Page");
		EXPECT_FALSE(env.formDirty());
		EXPECT_TRUE(env.formValid());
		EXPECT_TRUE(env.topMost());

		auto subPage = homePage->subpage("Page #1");
		EXPECT_FALSE(env.formDirty());
		EXPECT_TRUE(env.formValid());
		EXPECT_FALSE(env.topMost());

		subPage->setFormValid(false);
		EXPECT_FALSE(env.formValid());
		subPage->setFormValid(true);
		EXPECT_TRUE(env.formValid());
		subPage->setFormValid(false);
		EXPECT_FALSE(env.formValid());

		subPage->subpage("Page #2");
		EXPECT_TRUE(env.formValid());
		subPage->setFormValid(true);
		subPage->setFormValid(false);
		EXPECT_TRUE(env.formValid());
		subPage->setFormValid(false);
		EXPECT_TRUE(env.formValid());
	}

	TEST(Pages, mouseBack) {
		PagesEnv env{};

		auto homePage = env.stack->push<PagedWidgetMock>("Home Page");
		auto subPage = homePage->subpage("Page #1");
		subPage->subpage("Page #2");

		QMouseEvent evPress{QEvent::MouseButtonPress, QPointF{},      QPointF{},
		                    Qt::BackButton,           Qt::BackButton, Qt::NoModifier};
		QMouseEvent evRelease{
		    QEvent::MouseButtonRelease, QPointF{}, QPointF{}, Qt::BackButton, Qt::BackButton, Qt::NoModifier};

		auto const target = env.stack->page();

		qApp->notify(target, &evPress);
		EXPECT_EQ(target, env.stack->page());

		qApp->notify(target, &evRelease);
		EXPECT_NE(target, env.stack->page());
		EXPECT_EQ(subPage, env.stack->page());
	}
}  // namespace quick_dra::gui::testing
