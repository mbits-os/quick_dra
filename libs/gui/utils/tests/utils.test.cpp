// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <gtest/gtest.h>
#include <QWidget>
#include <app/utils/utils.hpp>
#include "TestApp.hpp"

using namespace std::literals;

namespace quick_dra::gui::testing {

	struct size_policy_testcase {
		QSizePolicy (*lazy)() = nullptr;
		std::string_view title{};
		QSizePolicy::Policy horizontal{QSizePolicy::Fixed};
		QSizePolicy::Policy vertical{QSizePolicy::Fixed};
		bool widthForHeight{false};
		bool heightForWidth{false};
		int horizontalStretch{0};
		int verticalStretch{0};

		friend std::ostream& operator<<(std::ostream& out, size_policy_testcase const& test) {
			return out << test.title;
		}
	};

	class utils : public ::testing::TestWithParam<size_policy_testcase> {};

	TEST(utils, restrictToolButton) {
		TestApp app{};
		QWidget wgt{};
		restrictToolButton(&wgt, 123);
		static constexpr QSize oneTwentyThree{123, 123};
#define EXPECT_SIZE_EQ(S1, S2)               \
	do {                                     \
		EXPECT_EQ(S1.width(), S2.width());   \
		EXPECT_EQ(S1.height(), S2.height()); \
	} while (false)
		EXPECT_SIZE_EQ(wgt.baseSize(), oneTwentyThree);
		EXPECT_SIZE_EQ(wgt.minimumSize(), oneTwentyThree);
		EXPECT_SIZE_EQ(wgt.maximumSize(), oneTwentyThree);
	}

	TEST_P(utils, size_policy) {
		auto const& param = GetParam();
		auto const actual = (*param.lazy)();
		EXPECT_EQ(actual.horizontalPolicy(), param.horizontal);
		EXPECT_EQ(actual.verticalPolicy(), param.vertical);
		EXPECT_EQ(actual.hasWidthForHeight(), param.widthForHeight);
		EXPECT_EQ(actual.hasHeightForWidth(), param.heightForWidth);
		EXPECT_EQ(actual.horizontalStretch(), param.horizontalStretch);
		EXPECT_EQ(actual.verticalStretch(), param.verticalStretch);
	}

#define LAZY(VALUE) +[]() -> QSizePolicy { return VALUE; }, .title = #VALUE ""sv

	static constexpr size_policy_testcase tests[] = {
	    {.lazy = LAZY(FixedSize)},
	    {.lazy = LAZY(TakeHeight), .vertical = QSizePolicy::Preferred},
	    {.lazy = LAZY(TakeWidth), .horizontal = QSizePolicy::Preferred},
	    {.lazy = LAZY(TakeAll), .horizontal = QSizePolicy::Preferred, .vertical = QSizePolicy::Preferred},
	    {.lazy = LAZY(TakeWidth / HeightForWidth), .horizontal = QSizePolicy::Preferred, .heightForWidth{true}},
	    {.lazy = LAZY(TakeWidth / HeightForWidth / 1_XStretch),
	     .horizontal = QSizePolicy::Preferred,
	     .heightForWidth{false},  // oops!
	     .horizontalStretch{1}},
	    {.lazy = LAZY(TakeWidth / (HeightForWidth / 1_XStretch)),
	     .horizontal = QSizePolicy::Preferred,
	     .heightForWidth{true},
	     .horizontalStretch{1}},
	    {.lazy = LAZY(TakeHeight / VerticalStretch(2)), .vertical = QSizePolicy::Preferred, .verticalStretch{2}},
	    {.lazy = LAZY(TakeHeight / 5_YStretch), .vertical = QSizePolicy::Preferred, .verticalStretch{5}},
	    {.lazy = LAZY(TakeWidth / HorizontalStretch(1)), .horizontal = QSizePolicy::Preferred, .horizontalStretch{1}},
	};

	INSTANTIATE_TEST_SUITE_P(tests, utils, ::testing::ValuesIn(tests));
}  // namespace quick_dra::gui::testing
