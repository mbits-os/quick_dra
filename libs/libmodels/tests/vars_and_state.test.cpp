// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <quick_dra/models/types.hpp>

namespace quick_dra::testing {
	TEST(varname, suffix) {
		ASSERT_EQ("single"_var, varname{.path = {"single"s}});
		ASSERT_EQ("parent.child"_var, (varname{.path = {"parent"s, "child"s}}));
	}

	TEST(varname, var_macros) {
		ASSERT_EQ(var::NN, "NN"_var);
		ASSERT_EQ(var::insured.document, "insured.document"_var);
		ASSERT_EQ(var::insured, "insured"_var);
		ASSERT_EQ(var::insured, varname{.path = {"insured"s}});
	}
	class global_object : public ::testing::Test {
	protected:
		quick_dra::global_object state_;

		void SetUp() override {
			state_.insert("values.currency"_var, 100_PLN);
			state_.insert("values.percent"_var, 23_per);
			state_.insert("values.string"_var, "a string"s);
			state_.insert("values.number"_var, uint_value{27});
			state_.insert("values.items"_var,
			              std::vector<calculated_value>{
			                  100_PLN, 200_PLN, "a string"s, uint_value{3}});
		}
	};

#define FIRST_ARG(A1, ...) A1
#define SECOND_ARG(A1, A2, ...) A2

#define ASSERT_STATE_OP(OP, name, ...)                                        \
	do {                                                                      \
		auto const expected_value = FIRST_ARG(__VA_ARGS__);                   \
		decltype(expected_value) default_value = SECOND_ARG(__VA_ARGS__, {}); \
		auto const actual_value = state_.typed_value(name, default_value);    \
		ASSERT_##OP(actual_value, expected_value);                            \
	} while (0)

#define ASSERT_STATE_EQ(name, ...) ASSERT_STATE_OP(EQ, name, __VA_ARGS__)
#define ASSERT_STATE_NE(name, ...) ASSERT_STATE_NE(EQ, name, __VA_ARGS__)

#define ASSERT_STATE_DEFAULTED(name, EXPECTED, ACTUAL)                     \
	do {                                                                   \
		auto const expected_value = EXPECTED;                              \
		decltype(expected_value) default_value = ACTUAL;                   \
		auto const actual_value = state_.typed_value(name, default_value); \
		ASSERT_NE(actual_value, expected_value);                           \
		ASSERT_EQ(actual_value, default_value);                            \
	} while (0)

	TEST_F(global_object, valid_reads) {
		ASSERT_STATE_EQ("values.currency"_var, 100_PLN);
		ASSERT_STATE_EQ("values.percent"_var, 23_per);
		ASSERT_STATE_EQ("values.string"_var, "a string"sv);
		ASSERT_STATE_EQ("values.number"_var, uint_value{27});
#if 0
        // REASON: get_typed<std::vector<calculated_value>> = delete
		ASSERT_STATE_EQ("values.items"_var, std::vector<calculated_value>{});
#endif
	}

	TEST_F(global_object, invalid_reads_currency) {
		ASSERT_STATE_DEFAULTED("no.such"_var, 100_PLN, 3.14_PLN);
		ASSERT_STATE_DEFAULTED("values.percent"_var, 100_PLN, 3.14_PLN);
		ASSERT_STATE_DEFAULTED("values.string"_var, 100_PLN, 3.14_PLN);
		ASSERT_STATE_DEFAULTED("values.number"_var, 100_PLN, 3.14_PLN);
		ASSERT_STATE_DEFAULTED("values.items"_var, 100_PLN, 3.14_PLN);
	}

	TEST_F(global_object, invalid_reads_percent) {
		ASSERT_STATE_DEFAULTED("no.such"_var, 23_per, -1_per);
		ASSERT_STATE_DEFAULTED("values.currency"_var, 23_per, -1_per);
		ASSERT_STATE_DEFAULTED("values.string"_var, 23_per, -1_per);
		ASSERT_STATE_DEFAULTED("values.number"_var, 23_per, -1_per);
		ASSERT_STATE_DEFAULTED("values.items"_var, 23_per, -1_per);
	}

	TEST_F(global_object, invalid_reads_string) {
		ASSERT_STATE_DEFAULTED("no.such"_var, "a string"s, "default str"s);
		ASSERT_STATE_DEFAULTED("values.currency"_var, "a string"s,
		                       "default str"s);
		ASSERT_STATE_DEFAULTED("values.percent"_var, "a string"s,
		                       "default str"s);
		ASSERT_STATE_DEFAULTED("values.number"_var, "a string"s,
		                       "default str"s);
		ASSERT_STATE_DEFAULTED("values.items"_var, "a string"s, "default str"s);
	}

	TEST_F(global_object, invalid_reads_string_view) {
		ASSERT_STATE_DEFAULTED("no.such"_var, "a string"sv,
		                       "default string_view"sv);
		ASSERT_STATE_DEFAULTED("values.currency"_var, "a string"sv,
		                       "default string_view"sv);
		ASSERT_STATE_DEFAULTED("values.percent"_var, "a string"sv,
		                       "default string_view"sv);
		ASSERT_STATE_DEFAULTED("values.number"_var, "a string"sv,
		                       "default string_view"sv);
		ASSERT_STATE_DEFAULTED("values.items"_var, "a string"sv,
		                       "default string_view"sv);
	}

	TEST_F(global_object, invalid_reads_C_str) {
		ASSERT_STATE_DEFAULTED("no.such"_var, "a string", "default C str");
		ASSERT_STATE_DEFAULTED("values.currency"_var, "a string",
		                       "default C str");
		ASSERT_STATE_DEFAULTED("values.percent"_var, "a string",
		                       "default C str");
		ASSERT_STATE_DEFAULTED("values.number"_var, "a string",
		                       "default C str");
		ASSERT_STATE_DEFAULTED("values.items"_var, "a string", "default C str");
	}
}  // namespace quick_dra::testing
