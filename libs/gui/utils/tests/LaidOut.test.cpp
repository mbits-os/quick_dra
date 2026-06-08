// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <gtest/gtest.h>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <app/utils/LaidOut.hpp>
#include <app/utils/utils.hpp>
#include "TestApp.hpp"

namespace quick_dra::gui::testing {
	TEST(LaidOut, widgetLayout) {
		TestApp app{};
		QWidget parent{};

		QHBoxLayout* rootLayout{};
		QVBoxLayout* verticalLayout{};
		QLabel* label{};

		LaidOut{&parent}.createLayout(rootLayout, "rootLayout", &parent);
		LaidOut{&parent}.withLayout(rootLayout).createLayout(verticalLayout, "verticalLayout", [](QBoxLayout& layout) {
			layout.setContentsMargins(0, 0, 0, 1);
			layout.setSpacing(112);
		});

		LaidOut{&parent, verticalLayout}.createWidget(label, "label",
		                                              [](QLabel& lbl) { setText(lbl, "Label text"sv); });

		auto const foundRoot = parent.findChild<QLayout*>("rootLayout");
		auto const foundVert = parent.findChild<QBoxLayout*>("verticalLayout");

		ASSERT_TRUE(rootLayout);
		ASSERT_EQ(rootLayout->objectName(), "rootLayout");
		ASSERT_EQ(foundRoot, rootLayout);
		ASSERT_EQ(rootLayout->parent(), &parent);

		ASSERT_TRUE(verticalLayout);
		ASSERT_EQ(verticalLayout->objectName(), "verticalLayout");
		ASSERT_EQ(foundVert, verticalLayout);
		ASSERT_EQ(rootLayout->findChild<QLayout*>("verticalLayout"), verticalLayout);
		ASSERT_EQ(verticalLayout->parent(), rootLayout);
		ASSERT_EQ(foundVert->contentsMargins(), QMargins(0, 0, 0, 1));
		ASSERT_EQ(foundVert->spacing(), 112);

		ASSERT_TRUE(label);
		ASSERT_EQ(label->objectName(), "label");
		ASSERT_EQ(label->parent(), &parent);
		ASSERT_EQ(label->text(), "Label text");

		setText(label, "Another text"sv);
		ASSERT_EQ(label->text(), "Another text");
	}

	TEST(LaidOut, gridLayout) {
		TestApp app{};
		QWidget parent{};

#define ROW(ROW_IDENTIFIER) \
	LaidOutGrid{            \
	    &parent,            \
	    rootLayout,         \
	    ROW_IDENTIFIER,     \
	}

		QGridLayout* rootLayout{};
		QLabel* label{};
		LaidOut{&parent}.createLayout(rootLayout, "rootLayout", &parent);

		ROW(0).createWidget(label, "first", {}).createWidget(label, "second", {.column = 1, .colSpan = 2});
		ROW(1).createWidget(label, "third", {.rowSpan = 2, .colSpan = 3});

		ASSERT_EQ(rootLayout->count(), 3);
		auto const first = rootLayout->itemAt(0);
		auto const second = rootLayout->itemAt(1);
		auto const third = rootLayout->itemAt(2);
		ASSERT_TRUE(first);
		ASSERT_TRUE(second);
		ASSERT_TRUE(third);
		ASSERT_EQ(first->widget()->objectName(), "first");
		ASSERT_EQ(second->widget()->objectName(), "second");
		ASSERT_EQ(third->widget()->objectName(), "third");

		auto refs = std::vector{
		    std::vector{first, second, second},
		    std::vector{third, third, third},
		    std::vector{third, third, third},
		};

		int row_number{-1};
		for (auto const& row : refs) {
			++row_number;
			int col_number{-1};
			for (auto const ptr : row) {
				++col_number;
				auto const actual = rootLayout->itemAtPosition(row_number, col_number);
				ASSERT_EQ(actual, ptr);
			}
		}
	}
}  // namespace quick_dra::gui::testing
