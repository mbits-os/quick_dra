// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QLabel>
#include "page_verify.hpp"

#include <app/pages/ReportFormPage.hpp>

using namespace quick_dra::gui;
using namespace quick_dra;

QLabel* find_label(QObject* parent, QAnyStringView text) {
	for (auto* child : parent->children()) {
		auto const label = qobject_cast<QLabel*>(child);
		if (label && label->text() == text) {
			return label;
		}
		auto result = find_label(child, text);
		if (result) return result;
	}
	return nullptr;
}

void verify_page(ReportFormPage& page) {
	QEvent pce{QEvent::PaletteChange};
	page.event(&pce);

	auto identifier = find_label(&page, "01 05-2020");
	QVERIFY(identifier);
	QCOMPARE_STR(identifier->toolTip(),
	             u"<div style=\"white-space:pre\"><b>Identyfikator raportu</b></div><div "
	             u"style=\"white-space:pre\"><i>\u00B7 numer / mm / rrrr</i></div>");

	QVERIFY_DOES_NOT_SURVIVE_RELOAD();
}
