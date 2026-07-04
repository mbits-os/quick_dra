// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "page_verify.hpp"

#include <app/pages/InsuredEditPage.hpp>
#include <app/pages/RemoveHistoryPage.hpp>

using namespace quick_dra::gui;
using namespace quick_dra;

namespace {
	static constexpr auto abmon = std::array{"sty"sv, "lut"sv, "mar"sv, "kwi"sv, "maj"sv, "cze"sv,
	                                         "lip"sv, "sie"sv, "wrz"sv, "paź"sv, "lis"sv, "gru"sv};
	QString nextMonth() {
		auto const today = get_today();
		auto const next = today.year() / today.month() + months{1};
		auto const result =
		    std::format("{} {}", abmon[static_cast<unsigned>(next.month()) - 1], static_cast<int>(next.year()));
		return QString::fromUtf8(result);
	}
}  // namespace

void insured_verify_page(RemoveHistoryPage& page);

void verify_page(InsuredEditPage& page) {
	QEvent pce{QEvent::PaletteChange};
	page.event(&pce);

	PARENT_CONTEXT(page);
	ENSURE_CHILD(QLineEdit, firstNameEdit);
	ENSURE_CHILD(QComboBox, documentKindComboBox);
	ENSURE_CHILD(QLineEdit, documentEdit);
	ENSURE_CHILD(QLabel, documentErrorLabel);
	ENSURE_CHILD(QTreeView, employmentHistoryTreeView);

	firstNameEdit->setText("");
	QVERIFY(page.isFormDirty());
	QVERIFY(!page.isFormValid());

	firstNameEdit->setText(QString::fromUtf16(u"Imię"));
	QVERIFY(page.isFormDirty());
	QVERIFY(page.isFormValid());

	documentKindComboBox->setCurrentIndex(documentKindComboBox->findData("P"));
	QCOMPARE_EQ(documentKindComboBox->currentData(), QString("P"));
	QVERIFY(page.isFormDirty());
	QVERIFY(!page.isFormValid());

	documentEdit->setText("78070707132");
	QVERIFY(page.isFormDirty());
	QVERIFY(!page.isFormValid());
	QCOMPARE_STR(documentErrorLabel->text(),
	             u"Antoni Kowalski: znaleziono inn\u0105 ubezpieczon\u0105 osob\u0119 z tym dokumentem");

	documentKindComboBox->setCurrentIndex(documentKindComboBox->findData("1"));

	page.addNewEmploymentHistoryEntry();
	DataSet expected{
	    {QString{"Od zawsze"}, QString{u"¼"}, QString{"Minimalna"}},
	    {nextMonth(), QString{"Pełny"}, QString{"Minimalna"}},
	};
	QCOMPARE_EQ(itemData(employmentHistoryTreeView->model(), Qt::DisplayRole), expected);
	// enum_object(&page, 0);

	page.removeEmploymentHistoryEntries();
	QVERIFY(qobject_cast<RemoveHistoryPage*>(PageStack::current()->page()));
	insured_verify_page(*qobject_cast<RemoveHistoryPage*>(PageStack::current()->page()));

	QVERIFY_ACCEPTS_STR(insured->front().first_name, u"Imię");
	QVERIFY_ACCEPTED_VAL(insured->front().history->size(), 1);
}

void insured_verify_page(RemoveHistoryPage& page) {
	PARENT_CONTEXT(page);
	ENSURE_CHILD(QCheckBox, selectAll);
	ENSURE_CHILD(QTreeView, historyList);

	auto const historyListModel = historyList->model();

	DataSet expected{
	    {QString{u"¼ minimalnej krajowej"}},
	    {QString{u"Minimalna krajowa (od %1)"}.arg(nextMonth())},
	};
	QCOMPARE_EQ(itemData(historyListModel, Qt::DisplayRole), expected);

	selectAll->setCheckState(Qt::Checked);
	historyList->activated(historyListModel->index(0, 0));

	QVERIFY_ACCEPTS();
}
