// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QDateEdit>
#include <QSignalSpy>
#include <QSpinBox>
#include "page_verify.hpp"

#include <app/pages/ReportIdEditPage.hpp>

using namespace quick_dra::gui;
using namespace quick_dra;

void verify_page(ReportIdEditPage& page) {
	page.initialData(Globals::current()->reportId());

	PARENT_CONTEXT(page);
	ENSURE_CHILD(QSpinBox, spinBox);
	ENSURE_CHILD(QDateEdit, dateEdit);
	ENSURE_CHILD(QToolButton, toolButton);

	page.setSerial(spinBox->value());
	spinBox->setValue(spinBox->value() + 1);
	QVERIFY(page.formDirty());

	spinBox->setValue(spinBox->value() - 1);
	QVERIFY(!page.formDirty());

	dateEdit->setDate(2020y / August / 10);
	QVERIFY(page.formDirty());
	QVERIFY(toolButton->isEnabled());

	toolButton->click();
	QVERIFY(!toolButton->isEnabled());
	dateEdit->setDate(2020y / August / 10);

	QVERIFY_SURVIVES_RELOAD();

	QSignalSpy spy{&page, &ReportIdEditPage::identifierUpdated};
	page.accept();

	QCOMPARE_NE(PageStack::current()->page(), &page);
	QCOMPARE_EQ(spy.size(), 1);
	QCOMPARE_EQ(spy[0], (QList<QVariant>{1, QDate{2020y / August / 1}, true}));
}
