// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <fmt/std.h>
#include <gtest/gtest.h>
#include <concepts>
#include <quick_dra/base/str.hpp>
#include <quick_dra/conv/low_level.hpp>
#include <sstream>

namespace quick_dra::testing {
	using std::literals::operator""s;
	using std::literals::operator""sv;

	template <std::predicate<std::istream&> Fn>
	std::pair<bool, std::string> captured(std::string_view input, Fn&& pred) {
		std::istringstream in{as_str(input)};

		::testing::internal::CaptureStdout();
		std::pair<bool, std::string> result{};

		result.first = pred(in);
		result.second = ::testing::internal::GetCapturedStdout();

		return result;
	}

	TEST(get_answer, hinted_answer) {
		auto const& [result, out] = captured(
		    R"(wrong
wrong
right)"sv,
		    [](auto& in) {
			    return get_answer("LABEL"sv, "HINT"sv, [](std::string&& value) { return value == "right"sv; }, in);
		    });

		EXPECT_TRUE(result);
		EXPECT_EQ(
		    out,
		    "\033[0;36mLABEL\033[0;90m [HINT]\033[m> \033[0;36mLABEL\033[0;90m [HINT]\033[m> \033[0;36mLABEL\033[0;90m [HINT]\033[m> "sv);
	}

	TEST(get_answer, un_hinted_answer) {
		auto const& [result, out] = captured(
		    R"(wrong
wrong)"sv,
		    [](auto& in) {
			    return get_answer("LABEL"sv, ""sv, [](std::string&& value) { return value == "right"sv; }, in);
		    });

		EXPECT_FALSE(result);
		EXPECT_EQ(out, "\033[0;36mLABEL\033[m> \033[0;36mLABEL\033[m> \033[0;36mLABEL\033[m> "sv);
	}

	TEST(get_yes_no, when_true_fail) {
		bool target{false};
		auto const& [result, out] =
		    captured(""sv, [&target](auto& in) { return get_yes_no("LABEL"sv, true, target, in); });
		EXPECT_FALSE(result);
		EXPECT_FALSE(target);
		EXPECT_EQ(out, "\033[0;36mLABEL\033[0;90m [Y/n]\033[m> "sv);
	}

	TEST(get_yes_no, when_true_success) {
		bool target{false};
		auto const& [result, out] =
		    captured("NO\nNO\nY\nNO\n"sv, [&target](auto& in) { return get_yes_no("LABEL"sv, true, target, in); });
		EXPECT_TRUE(result);
		EXPECT_TRUE(target);
		EXPECT_EQ(out,
		          "\033[0;36mLABEL\033[0;90m [Y/n]\033[m> "
		          "\033[0;36mLABEL\033[0;90m [Y/n]\033[m> "
		          "\033[0;36mLABEL\033[0;90m [Y/n]\033[m> "sv);

		target = false;
		auto const& [result2, out2] =
		    captured("y\n"sv, [&target](auto& in) { return get_yes_no("LABEL"sv, true, target, in); });
		EXPECT_TRUE(result2);
		EXPECT_TRUE(target);
		EXPECT_EQ(out2, "\033[0;36mLABEL\033[0;90m [Y/n]\033[m> "sv);
	}

	TEST(get_yes_no, when_false_success) {
		bool target{true};
		auto const& [result, out] =
		    captured("NO\nNO\nN\nNO\n"sv, [&target](auto& in) { return get_yes_no("LABEL"sv, false, target, in); });
		EXPECT_TRUE(result);
		EXPECT_FALSE(target);
		EXPECT_EQ(out,
		          "\033[0;36mLABEL\033[0;90m [y/N]\033[m> "
		          "\033[0;36mLABEL\033[0;90m [y/N]\033[m> "
		          "\033[0;36mLABEL\033[0;90m [y/N]\033[m> "sv);

		target = true;
		auto const& [result2, out2] =
		    captured("n\n"sv, [&target](auto& in) { return get_yes_no("LABEL"sv, false, target, in); });
		EXPECT_TRUE(result2);
		EXPECT_FALSE(target);
		EXPECT_EQ(out2, "\033[0;36mLABEL\033[0;90m [y/N]\033[m> "sv);

		target = true;
		auto const& [result3, out3] =
		    captured("\n"sv, [&target](auto& in) { return get_yes_no("LABEL"sv, false, target, in); });
		EXPECT_TRUE(result3);
		EXPECT_FALSE(target);
		EXPECT_EQ(out3, "\033[0;36mLABEL\033[0;90m [y/N]\033[m> "sv);
	}

	TEST(get_enum_answer, no_selection) {
		char target{0};
		auto const conv = [&target](char ch) { target = ch; };
		auto const& [result, out] = captured("\n22\n2\n"sv, [conv](auto& in) {
			return get_enum_answer("LABEL"sv, std::vector{std::pair{'1', "ONE"sv}, std::pair{'2', "TWO"sv}}, conv, 0,
			                       in);
		});
		EXPECT_TRUE(result);
		EXPECT_EQ(target, '2');
		EXPECT_EQ(out,
		          "\033[0;36mLABEL\033[0;90m [1 - ONE, 2 - TWO]\033[m> "
		          "\033[0;36mLABEL\033[0;90m [1 - ONE, 2 - TWO]\033[m> "
		          "\033[0;36mLABEL\033[0;90m [1 - ONE, 2 - TWO]\033[m> "sv);
	}

	TEST(get_enum_answer, with_selection) {
		char target{0};
		auto const conv = [&target](char ch) { target = ch; };
		auto const& [result, out] = captured("22\n\n"sv, [conv](auto& in) {
			return get_enum_answer("LABEL"sv, std::vector{std::pair{'1', "ONE"sv}, std::pair{'2', "TWO"sv}}, conv, '2',
			                       in);
		});
		EXPECT_TRUE(result);
		EXPECT_EQ(target, '2');
		EXPECT_EQ(out,
		          "\033[0;36mLABEL\033[0;90m [1 - ONE, [2] - TWO]\033[m> "
		          "\033[0;36mLABEL\033[0;90m [1 - ONE, [2] - TWO]\033[m> "sv);
	}

	TEST(get_enum_answer, nothing_selected) {
		char target{0};
		auto const conv = [&target](char ch) { target = ch; };
		auto const& [result, out] = captured("22\nP\n"sv, [conv](auto& in) {
			return get_enum_answer("LABEL"sv, std::vector{std::pair{'1', "ONE"sv}, std::pair{'2', "TWO"sv}}, conv, '2',
			                       in);
		});
		EXPECT_FALSE(result);
		EXPECT_EQ(target, '\0');
		EXPECT_EQ(out,
		          "\033[0;36mLABEL\033[0;90m [1 - ONE, [2] - TWO]\033[m> "
		          "\033[0;36mLABEL\033[0;90m [1 - ONE, [2] - TWO]\033[m> "
		          "\033[0;36mLABEL\033[0;90m [1 - ONE, [2] - TWO]\033[m> "sv);
	}

	TEST(as_string, std_string) {
		auto const input = "input"s;
		auto const& output = as_string(input);
		EXPECT_EQ(std::addressof(input), std::addressof(output));
	}

	TEST(as_string, std_string_refref) {
		auto input = "input"s;
		auto&& output = as_string(std::move(input));
		EXPECT_EQ(std::addressof(input), std::addressof(output));
	}
}  // namespace quick_dra::testing
