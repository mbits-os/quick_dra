// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "page_verify.hpp"

#include <app/pages/PayerEditPage.hpp>

using namespace quick_dra::gui;
using namespace quick_dra;

void verify_page(PayerEditPage& page) {
	QEvent pce{QEvent::PaletteChange};
	page.event(&pce);

	PARENT_CONTEXT(page);
	ENSURE_CHILD(QLineEdit, firstNameEdit);

	auto const previous = firstNameEdit->text();
	firstNameEdit->setText("");
	QVERIFY(page.isFormDirty());
	QVERIFY(!page.isFormValid());

	firstNameEdit->setText("Word");
	QVERIFY(page.isFormDirty());
	QVERIFY(page.isFormValid());

	firstNameEdit->setText(previous);
	QVERIFY(!page.isFormDirty());
	QVERIFY(page.isFormValid());

	firstNameEdit->setText(QString::fromUtf16(u"Imię"));
	QVERIFY(page.isFormDirty());
	QVERIFY(page.isFormValid());

	QVERIFY_ACCEPTS_STR(payer->first_name, u"Imię");
}
