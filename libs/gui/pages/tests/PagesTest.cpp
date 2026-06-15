// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "PagesTest.hpp"
#include <app/gui/PageHeader.hpp>
#include <app/gui/PageStack.hpp>
#include <app/gui/PagedWidget.hpp>
#include "TestPaths.hpp"
#include "test_offscreen.hpp"
#include "ui_helpers.hpp"

#include <app/pages/HomePage.hpp>
#include <app/pages/InsuredEditPage.hpp>
#include <app/pages/PayerEditPage.hpp>
#include <app/pages/PersonelPage.hpp>
#include <app/pages/RemoveHistoryPage.hpp>
#include <app/pages/RemoveInsuredPage.hpp>
#include <app/pages/ReportFormPage.hpp>
#include <app/pages/ReportIdEditPage.hpp>

QTEST_OFFSCREEN_MAIN(PagesTest)

using namespace quick_dra::gui;
using namespace quick_dra;

static constexpr size_t first_pos = 0;

#define PAGE_TYPES(X)             \
	X(HomePage)                   \
	X(ReportIdEditPage)           \
	X(PersonelPage)               \
	X(ReportFormPage, first_pos)  \
	X(PayerEditPage)              \
	X(InsuredEditPage, first_pos) \
	X(RemoveInsuredPage);

namespace quick_dra::gui {
#define FFW(NAME, ...) class NAME;
	PAGE_TYPES(FFW)
#undef FFW
};  // namespace quick_dra::gui

static auto cliTestDataDir() { return std::filesystem::path{testing::cli_test_data}; }

using PagePushFunction = std::function<void(PageStack&)>;
Q_DECLARE_METATYPE(PagePushFunction)

using PageVerifyFunction = std::function<void(PagedWidget&)>;
Q_DECLARE_METATYPE(PageVerifyFunction)

template <typename PageType, typename... Args>
PagePushFunction push_page_function(Args&&... args) {
	return [args...](PageStack& stack) { stack.push<PageType>(args...); };
}

#define DELCARE_VERIFY_PAGE(NAME, ...) void verify_page(NAME&);
PAGE_TYPES(DELCARE_VERIFY_PAGE)
#undef VERIFY_PAGE

template <typename PageType>
PageVerifyFunction page_verify_function() {
	return [](PagedWidget& page) { verify_page(static_cast<PageType&>(page)); };
}

void PagesTest::pushPage_data() {
	QTest::addColumn<PagePushFunction>("pagePushFunction");
	QTest::addColumn<PageVerifyFunction>("pageVerifyFunction");
#define TEST_ROW(NAME, ...) \
	QTest::addRow(#NAME) << push_page_function<NAME>(__VA_ARGS__) << page_verify_function<NAME>();
	PAGE_TYPES(TEST_ROW)
}

void PagesTest::pushPage() {
	TmpDir tmp{};
	platform::config_data_dir(testing::form_data_config);
	writeConfig(tmp.cwd(), cliTestDataDir());
	Globals globals{SettingsProvider::wrap([&tmp] { return openSettings(tmp.cwd()); })};
	QStackedWidget stackedWidget{};
	auto header = new PageHeader{&stackedWidget};
	auto stack = new PageStack{header, &stackedWidget};
	globals.setStack(stack);
	globals.setConfig(tmp.cwd() / ".quick_dra.yaml"sv, std::nullopt, false);

	QFETCH(PagePushFunction, pagePushFunction);
	QFETCH(PageVerifyFunction, pageVerifyFunction);

	stack->push<PagedWidget>();  // for navigateHomeForReload tests to work
	pagePushFunction(*stack);
	pageVerifyFunction(*stack->page());
}
