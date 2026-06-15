// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "page_verify.hpp"

#include <app/pages/RemoveInsuredPage.hpp>

using namespace quick_dra::gui;
using namespace quick_dra;

void verify_page(RemoveInsuredPage& page) {
	QVERIFY_CHILD(QCheckBox, selectAll);
	QVERIFY_CHILD(QTreeView, insuredList);

	auto const insuredListModel = insuredList->model();

	DataSet expected{
	    {QString{u"Piotr Iksiński (dowód osobisty: ABC523456)"}},
	    {QString{u"Antoni Kowalski (PESEL: 78070707132)"}},
	    {QString{u"Maria Iksińska (PESEL: 26211012346)"}},
	    {QString{u"Antonia Iksińska (PESEL: 00000000000)"}},
	};
	QCOMPARE_EQ(itemData(insuredListModel, Qt::DisplayRole), expected);
	QVERIFY_SURVIVES_RELOAD();

	selectAll->setCheckState(Qt::Checked);
	insuredList->activated(insuredListModel->index(1, 0));
	insuredList->activated(insuredListModel->index(3, 0));

	QCOMPARE_EQ(selectAll->checkState(), Qt::PartiallyChecked);

	QVERIFY_ACCEPTS();
	QVERIFY_ACCEPTED_VAL(insured->size(), 2);
}
