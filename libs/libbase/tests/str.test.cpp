// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <quick_dra/base/str.hpp>

namespace quick_dra::testing {
	TEST(str, conv) {
		ASSERT_EQ(as_u8v("NCC-1031"sv), u8"NCC-1031"sv);
		ASSERT_EQ(as_sv(u8"NCC-1031"sv), "NCC-1031"sv);
		ASSERT_EQ(as_str(u8"NCC-1031"sv), "NCC-1031"s);
		ASSERT_EQ(as_str("NCC-1031"sv), "NCC-1031"s);
	}

	TEST(str, split_sv) {
		auto actual = split_sv("A:B:C"sv, ':'_sep);
		auto expected = std::vector{"A"sv, "B"sv, "C"sv};
		ASSERT_EQ(actual, expected);

		actual = split_sv("A:B:C"sv, ':'_sep, 1);
		expected = std::vector{"A"sv, "B:C"sv};
		ASSERT_EQ(actual, expected);

		actual = split_sv("A:B:C"sv, " :: "_sep);
		expected = std::vector{"A:B:C"sv};
		ASSERT_EQ(actual, expected);

		actual = split_sv("A :: B :: C"sv, " :: "_sep);
		expected = std::vector{"A"sv, "B"sv, "C"sv};
		ASSERT_EQ(actual, expected);

		auto str_ref = "A:B:C"s;
		expected = std::vector{"A"sv, "B"sv, "C"sv};

		actual = split_sv(str_ref, ':'_sep);
		ASSERT_EQ(actual, expected);

		actual = split_sv(str_ref, ":"_sep);
		ASSERT_EQ(actual, expected);
	}

	TEST(str, split_s) {
		auto actual = split_s("A:B:C"sv, ':'_sep);
		auto expected = std::vector{"A"s, "B"s, "C"s};
		ASSERT_EQ(actual, expected);

		actual = split_s("A:B:C"sv, ':'_sep, 1);
		expected = std::vector{"A"s, "B:C"s};
		ASSERT_EQ(actual, expected);

		actual = split_s("A:B:C"sv, " :: "_sep);
		expected = std::vector{"A:B:C"s};
		ASSERT_EQ(actual, expected);

		actual = split_s("A :: B :: C"sv, " :: "_sep);
		expected = std::vector{"A"s, "B"s, "C"s};
		ASSERT_EQ(actual, expected);
	}

	TEST(str, split_on_empty) {
		auto actual = split_sv("\t\v\r\nA\tB C\t\tD\nE"sv, ""_sep);
		auto expected = std::vector{"A"sv, "B"sv, "C"sv, "D"sv, "E"sv};
		ASSERT_EQ(actual, expected);

		actual = split_sv("\t\v\r\n\t \t\t\n"sv, ""_sep);
		expected = std::vector{""sv};
		ASSERT_EQ(actual, expected);
	}

	TEST(str, strip_sv) {
		ASSERT_EQ(strip_sv("  text \n text \t\n\r"sv), "text \n text"sv);
		ASSERT_EQ(rstrip_sv("  text \n text \t\n\r"sv), "  text \n text"sv);
		ASSERT_EQ(lstrip_sv("  text \n text \t\n\r"sv),
		          "text \n text \t\n\r"sv);
	}

	TEST(str, strip_s) {
		ASSERT_EQ(strip_s("  text \n text \t\n\r"sv), "text \n text"s);
		ASSERT_EQ(rstrip_s("  text \n text \t\n\r"sv), "  text \n text"s);
		ASSERT_EQ(lstrip_s("  text \n text \t\n\r"sv), "text \n text \t\n\r"s);
	}

	TEST(str, join) {
		std::vector sv_lines = {"one"sv, "two"sv, "three"sv};
		std::vector str_lines = {"one"s, "two"s, "three"s};

		ASSERT_EQ(join(sv_lines, '.'_sep), "one.two.three"sv);
		ASSERT_EQ(join(str_lines, '/'_sep), "one/two/three"sv);
		ASSERT_EQ(join(sv_lines, " >> "_sep), "one >> two >> three"sv);
		ASSERT_EQ(join(str_lines, ":"_sep), "one:two:three"sv);
	}
}  // namespace quick_dra::testing
