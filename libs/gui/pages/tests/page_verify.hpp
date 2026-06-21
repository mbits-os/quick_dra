// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QTest>
#include "ui_helpers.hpp"

#define QCOMPARE_STR(lhs, rhs)                                                                               \
	do {                                                                                                     \
		QString s1{lhs};                                                                                     \
		QString s2{rhs};                                                                                     \
		if (!QTest::qCompareOp<QTest::ComparisonOperation::Equal>(s1, s2, #lhs, #rhs, __FILE__, __LINE__)) { \
			QTEST_FAIL_ACTION;                                                                               \
		}                                                                                                    \
	} while (false)

#define QVERIFY_NAVIGATION(ACTION, TYPE, TITLE)                           \
	do {                                                                  \
		ACTION;                                                           \
		QVERIFY(qobject_cast<TYPE*>(PageStack::current()->page()));       \
		QCOMPARE_STR(PageStack::current()->page()->windowTitle(), TITLE); \
		PageStack::current()->navigateBack();                             \
	} while (false)

#define QVERIFY_SURVIVES_RELOAD()                         \
	do {                                                  \
		QVERIFY(page.survivesReload());                   \
		PageStack::current()->navigateHomeForReload();    \
		QCOMPARE_EQ(PageStack::current()->page(), &page); \
	} while (false)

#define QVERIFY_DOES_NOT_SURVIVE_RELOAD()                 \
	do {                                                  \
		QVERIFY(!page.survivesReload());                  \
		PageStack::current()->navigateHomeForReload();    \
		QCOMPARE_NE(PageStack::current()->page(), &page); \
	} while (false)

#define QVERIFY_ACCEPTS()           \
	do {                            \
		page.accept();              \
                                    \
		QVERIFY(!page.formDirty()); \
		QVERIFY(page.formValid());  \
	} while (false)

#define QVERIFY_ACCEPTED_STR(FIELD, VALUE) \
	QCOMPARE_STR(QString::fromUtf8(*Globals::current()->data().cfg.FIELD), VALUE);

#define QVERIFY_ACCEPTED_VAL(FIELD, VALUE) QCOMPARE_EQ(Globals::current()->data().cfg.FIELD, VALUE);
#define QVERIFY_ACCEPTED(FIELD) QVERIFY(Globals::current()->data().cfg.FIELD);

#define QVERIFY_ACCEPTS_STR(FIELD, VALUE)   \
	do {                                    \
		QVERIFY_ACCEPTS();                  \
		QVERIFY_ACCEPTED_STR(FIELD, VALUE); \
	} while (false)
