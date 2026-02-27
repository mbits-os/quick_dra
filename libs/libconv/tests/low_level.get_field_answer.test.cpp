// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <fmt/std.h>
#include <gtest/gtest.h>
#include <concepts>
#include <quick_dra/base/str.hpp>
#include <quick_dra/conv/low_level.hpp>
#include <quick_dra/conv/validators.hpp>
#include <quick_dra/lex/validators.hpp>
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

	TEST(get_field_answer, currency_bad) {
		{
			auto const& [result, out] = captured("15 $"sv, [](auto& in) {
				std::optional<currency> dst{};
				return get_field_answer(true, "LABEL"sv, dst, std::nullopt,
				                        builtin::valid_salary, in);
			});

			EXPECT_FALSE(result);
			EXPECT_EQ(out,
			          "\033[0;36mLABEL\033[0;90m [none]\033[m> "
			          "\033[0;90m - The monetary value provided seems to be "
			          "invalid.\033[m\n"
			          "\033[0;36mLABEL\033[0;90m [none]\033[m> "sv);
		}

		{
			auto const& [result, out] = captured(""sv, [](auto& in) {
				std::optional<currency> dst{};
				return get_field_answer(true, "LABEL"sv, dst, std::nullopt,
				                        builtin::valid_salary, in);
			});

			EXPECT_FALSE(result);
			EXPECT_EQ(out, "\033[0;36mLABEL\033[0;90m [none]\033[m> "sv);
		}

		{
			auto const& [result, out] = captured(""sv, [](auto& in) {
				std::optional<currency> dst{};
				return get_field_answer(false, "LABEL"sv, dst, std::nullopt,
				                        builtin::valid_salary, in);
			});

			EXPECT_FALSE(result);
			EXPECT_EQ(
			    out,
			    "\033[0;90m - The monetary value provided seems to be "
			    "invalid.\033[m\n"
			    "\033[0;90m - Cannot save invalid data with -y. Stopping.\033[m\n"sv);
		}

		{
			auto const& [result, out] = captured(""sv, [](auto& in) {
				std::optional<currency> dst{};
				return get_field_answer(
				    false, "LABEL"sv, dst, 23.45_PLN,
				    [](std::string&&, std::optional<currency>&, bool) -> bool {
					    return false;
				    },
				    in);
			});

			EXPECT_FALSE(result);
			EXPECT_EQ(
			    out,
			    "\033[0;90m - Cannot save invalid data with -y. Stopping.\033[m\n"sv);
		}
	}

	TEST(get_field_answer, currency_good) {
		std::optional<currency> dst{};
		{
			dst.reset();
			auto const& [result, out] = captured("15"sv, [&dst](auto& in) {
				return get_field_answer(true, "LABEL"sv, dst, std::nullopt,
				                        builtin::valid_salary, in);
			});

			EXPECT_TRUE(result);
			EXPECT_EQ(dst, std::optional{15_PLN});
			EXPECT_EQ(out, "\033[0;36mLABEL\033[0;90m [none]\033[m> "sv);
		}

		{
			dst.reset();
			auto const& [result, out] = captured("15 zł\n"sv, [&dst](auto& in) {
				return get_field_answer(true, "LABEL"sv, dst, 23.45_PLN,
				                        builtin::valid_salary, in);
			});

			EXPECT_TRUE(result);
			EXPECT_EQ(dst, std::optional{15_PLN});
			EXPECT_EQ(out, "\033[0;36mLABEL\033[0;90m [23.45 zł]\033[m> "sv);
		}

		{
			dst.reset();
			auto const& [result, out] = captured("\n"sv, [&dst](auto& in) {
				return get_field_answer(true, "LABEL"sv, dst, minimal_salary,
				                        builtin::valid_salary, in);
			});

			EXPECT_TRUE(result);
			EXPECT_EQ(dst, std::optional{minimal_salary});
			EXPECT_EQ(
			    out,
			    "\033[0;36mLABEL\033[0;90m [minimal for a given month]\033[m> "sv);
		}

		{
			dst.reset();
			auto const& [result, out] = captured("15\n"sv, [&dst](auto& in) {
				return get_field_answer(false, "LABEL"sv, dst, 23.45_PLN,
				                        builtin::valid_salary, in);
			});

			EXPECT_TRUE(result);
			EXPECT_EQ(dst, std::optional{23.45_PLN});
			EXPECT_EQ(out, ""sv);
		}
	}

	TEST(get_field_answer, ratio_bad) {
		{
			auto const& [result, out] = captured("badf00d"sv, [](auto& in) {
				std::optional<ratio> dst{};
				return get_field_answer(true, "LABEL"sv, dst, std::nullopt,
				                        builtin::valid_part_time_scale, in);
			});

			EXPECT_FALSE(result);
			EXPECT_EQ(out,
			          "\033[0;36mLABEL\033[m> "
			          "\033[0;90m - The ratio value provided seems to be "
			          "invalid.\033[m\n"
			          "\033[0;36mLABEL\033[m> "sv);
		}

		{
			auto const& [result, out] = captured("badf00d"sv, [](auto& in) {
				std::optional<ratio> dst{};
				return get_field_answer(true, "LABEL"sv, dst, ratio{3, 4},
				                        builtin::valid_part_time_scale, in);
			});

			EXPECT_FALSE(result);
			EXPECT_EQ(out,
			          "\033[0;36mLABEL\033[0;90m [3/4]\033[m> "
			          "\033[0;90m - The ratio value provided seems to be "
			          "invalid.\033[m\n"
			          "\033[0;36mLABEL\033[0;90m [3/4]\033[m> "sv);
		}

		{
			auto const& [result, out] = captured(""sv, [](auto& in) {
				std::optional<ratio> dst{};
				return get_field_answer(
				    false, "LABEL"sv, dst, ratio{3, 4},
				    [](std::string&&, std::optional<ratio>&, bool) {
					    return false;
				    },
				    in);
			});

			EXPECT_FALSE(result);
			EXPECT_EQ(
			    out,
			    "\033[0;90m - Cannot save invalid data with -y. Stopping.\033[m\n"sv);
		}
	}

	TEST(get_field_answer, ratio_good) {
		std::optional<ratio> dst{};
		{
			auto const& [result, out] = captured("2/5"sv, [&dst](auto& in) {
				return get_field_answer(true, "LABEL"sv, dst, std::nullopt,
				                        builtin::valid_part_time_scale, in);
			});

			EXPECT_TRUE(result);
			EXPECT_EQ(dst, std::optional{ratio{2 / 5}});
			EXPECT_EQ(out, "\033[0;36mLABEL\033[m> "sv);
		}

		{
			auto const& [result, out] = captured("\n"sv, [&dst](auto& in) {
				return get_field_answer(true, "LABEL"sv, dst, ratio{3, 4},
				                        builtin::valid_part_time_scale, in);
			});

			EXPECT_TRUE(result);
			EXPECT_EQ(dst, std::optional{ratio{3 / 4}});
			EXPECT_EQ(out, "\033[0;36mLABEL\033[0;90m [3/4]\033[m> "sv);
		}

		{
			auto const& [result, out] = captured(""sv, [&dst](auto& in) {
				return get_field_answer(false, "LABEL"sv, dst, ratio{3, 4},
				                        builtin::valid_part_time_scale, in);
			});

			EXPECT_TRUE(result);
			EXPECT_EQ(dst, std::optional{ratio{3 / 4}});
			EXPECT_EQ(out, ""sv);
		}
	}

	TEST(get_field_answer, insurance_title_bad) {
		{
			auto const& [result, out] = captured("badf00d"sv, [](auto& in) {
				std::optional<insurance_title> dst{};
				return get_field_answer(true, "LABEL"sv, dst, std::nullopt,
				                        builtin::valid_title, in);
			});

			EXPECT_FALSE(result);
			EXPECT_EQ(
			    out,
			    "\033[0;36mLABEL\033[m> "
			    "\033[0;90m - The insurance title value provided seems to be "
			    "invalid.\033[m\n"
			    "\033[0;36mLABEL\033[m> "sv);
		}

		{
			auto const& [result, out] = captured("badf00d"sv, [](auto& in) {
				std::optional<insurance_title> dst{};
				return get_field_answer(true, "LABEL"sv, dst,
				                        insurance_title{"0000", 0, 0},
				                        builtin::valid_title, in);
			});

			EXPECT_FALSE(result);
			EXPECT_EQ(
			    out,
			    "\033[0;36mLABEL\033[0;90m [0000 0 0]\033[m> "
			    "\033[0;90m - The insurance title value provided seems to be "
			    "invalid.\033[m\n"
			    "\033[0;36mLABEL\033[0;90m [0000 0 0]\033[m> "sv);
		}

		{
			auto const& [result, out] = captured(""sv, [](auto& in) {
				std::optional<insurance_title> dst{};
				return get_field_answer(
				    false, "LABEL"sv, dst, insurance_title{"0000", 0, 0},
				    [](std::string&&, std::optional<insurance_title>&, bool) {
					    return false;
				    },
				    in);
			});

			EXPECT_FALSE(result);
			EXPECT_EQ(
			    out,
			    "\033[0;90m - Cannot save invalid data with -y. Stopping.\033[m\n"sv);
		}
	}

	TEST(get_field_answer, insurance_title_good) {
		std::optional<insurance_title> dst{};
		{
			auto const& [result, out] =
			    captured("4444 5 6"sv, [&dst](auto& in) {
				    return get_field_answer(true, "LABEL"sv, dst, std::nullopt,
				                            builtin::valid_title, in);
			    });

			EXPECT_TRUE(result);
			EXPECT_EQ(dst, (std::optional{insurance_title{"4444", 5, 6}}));
			EXPECT_EQ(out, "\033[0;36mLABEL\033[m> "sv);
		}

		{
			auto const& [result, out] = captured("\n"sv, [&dst](auto& in) {
				return get_field_answer(true, "LABEL"sv, dst,
				                        insurance_title{"0000", 0, 0},
				                        builtin::valid_title, in);
			});

			EXPECT_TRUE(result);
			EXPECT_EQ(dst, (std::optional{insurance_title{"0000", 0, 0}}));
			EXPECT_EQ(out, "\033[0;36mLABEL\033[0;90m [0000 0 0]\033[m> "sv);
		}

		{
			auto const& [result, out] = captured(""sv, [&dst](auto& in) {
				return get_field_answer(false, "LABEL"sv, dst,
				                        insurance_title{"0000", 0, 0},
				                        builtin::valid_title, in);
			});

			EXPECT_TRUE(result);
			EXPECT_EQ(dst, (std::optional{insurance_title{"0000", 0, 0}}));
			EXPECT_EQ(out, ""sv);
		}
	}

	TEST(get_field_answer, string_validators) {
		{
			std::optional<std::string> dst{};
			auto const& [result, out] = captured(""sv, [&dst](auto& in) {
				return get_field_answer(false, "LABEL"sv, dst, ""s,
				                        builtin::valid_last_name, in);
			});

			EXPECT_FALSE(result);
			EXPECT_EQ(
			    out,
			    "\033[0;90m - Cannot save invalid data with -y. Stopping.\033[m\n"sv);
		}
#define ENSURE_VALID_INPUT_EX(CODE, VALIDATOR, VALIDATED)                 \
	{                                                                     \
		std::optional<std::string> interactive_dst{};                     \
		auto const& [result, out] =                                       \
		    captured(CODE##sv, [&interactive_dst](auto& in) {             \
			    return get_field_answer(true, "LABEL"sv, interactive_dst, \
			                            std::nullopt, VALIDATOR, in);     \
		    });                                                           \
                                                                          \
		EXPECT_TRUE(result);                                              \
		EXPECT_EQ(interactive_dst, VALIDATED##s);                         \
		EXPECT_EQ(out, "\033[0;36mLABEL\033[m> "sv);                      \
	}                                                                     \
                                                                          \
	{                                                                     \
		std::optional<std::string> yes_dst{};                             \
		auto const& [result, out] = captured(""sv, [&yes_dst](auto& in) { \
			return get_field_answer(false, "LABEL"sv, yes_dst, CODE##s,   \
			                        VALIDATOR, in);                       \
		});                                                               \
                                                                          \
		EXPECT_TRUE(result);                                              \
		EXPECT_EQ(yes_dst, VALIDATED##s);                                 \
		EXPECT_EQ(out, ""sv);                                             \
	}

#define ENSURE_VALID_INPUT(CODE, VALIDATOR) \
	ENSURE_VALID_INPUT_EX(CODE, VALIDATOR, CODE)

		ENSURE_VALID_INPUT("name", builtin::valid_first_name);
		ENSURE_VALID_INPUT("26211012346", builtin::valid_social_id);
		ENSURE_VALID_INPUT("AAA000000", builtin::valid_id_card);
		ENSURE_VALID_INPUT("AA0000000", builtin::valid_passport);
		ENSURE_VALID_INPUT("7680002466", builtin::valid_tax_id);
		ENSURE_VALID_INPUT_EX("768-000-24-66", builtin::valid_tax_id,
		                      "7680002466");
		ENSURE_VALID_INPUT("xyxzy",
		                   builtin::policies::always_valid<std::string>);

#define ENSURE_INVALID_INPUT(CODE, VALIDATOR, ERROR)                           \
	{                                                                          \
		std::optional<std::string> dst{};                                      \
		auto const& [result, out] = captured(""sv, [&dst](auto& in) {          \
			return get_field_answer(false, "LABEL"sv, dst, CODE##s, VALIDATOR, \
			                        in);                                       \
		});                                                                    \
                                                                               \
		EXPECT_FALSE(result);                                                  \
		EXPECT_EQ(out, "\033[0;90m - The " ERROR                               \
		               " provided seems to be invalid.\033[m\n"                \
		               "\033[0;90m - Cannot save invalid data with -y. "       \
		               "Stopping.\033[m\n"sv);                                 \
	}

		ENSURE_INVALID_INPUT("99999999999", builtin::valid_social_id,
		                     "social id");
		ENSURE_INVALID_INPUT("BBB999999", builtin::valid_id_card, "ID card");
		ENSURE_INVALID_INPUT("BB9999999", builtin::valid_passport,
		                     "passport number");
		ENSURE_INVALID_INPUT("9898989777", builtin::valid_tax_id, "tax id");
	}
}  // namespace quick_dra::testing
