// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "page_verify.hpp"

#include <app/pages/InsuredEditPage.hpp>
#include <app/pages/PayerEditPage.hpp>
#include <app/pages/PersonelPage.hpp>
#include <app/pages/RemoveInsuredPage.hpp>

using namespace quick_dra::gui;
using namespace quick_dra;

void verify_page(PersonelPage& page) {
	auto group = page.insuredButtons();
	auto const last = group->count() ? group->itemAt(group->count() - 1) : nullptr;
	QVERIFY(last && last->isClickable());

	QVERIFY_NAVIGATION(page.editPayer(), PayerEditPage, u"Jan Nowak (P\u0142atnik)");
	QVERIFY_NAVIGATION(page.editInsured(0), InsuredEditPage, u"Piotr Iksi\u0144ski (Ubezpieczony)");
	QVERIFY_NAVIGATION(last->clicked(), InsuredEditPage, u"Antonia Iksi\u0144ska (Ubezpieczony)");
	QVERIFY_NAVIGATION(page.addInsured(), InsuredEditPage, u"<Nieznany ubezpieczony> (Ubezpieczony)");
	QVERIFY_NAVIGATION(page.removeInsured(), RemoveInsuredPage, u"Wybierz pozycje do usuni\u0119cia");

	QVERIFY_SURVIVES_RELOAD();
}
