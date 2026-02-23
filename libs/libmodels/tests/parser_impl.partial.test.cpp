// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "parser_impl.common.hpp"

namespace quick_dra::testing {
	class partial_parser_impl : public ::testing::Test,
	                            public object_reader<partial::config> {};

	TEST_F(partial_parser_impl, no_last_name) {
		auto value = read(R"(płatnik: {})"sv);
		ASSERT_TRUE(value);
		ASSERT_TRUE(value->payer);
		ASSERT_FALSE(value->payer->first_name);
		ASSERT_FALSE(value->payer->last_name);
		value->payer->first_name = "first"s;
		value->payer->last_name = "last"s;
		value->prepare_for_write();
		auto const expected_output = R"(wersja: 1
płatnik:
  nazwisko: 'last, first'
ubezpieczeni: []
)"sv;
		auto const actual_output = write(*value);
		ASSERT_EQ(actual_output, expected_output);
	}

	TEST_F(partial_parser_impl, only_last_name) {
		auto value = read(R"(wersja: 1
płatnik:
  nazwisko: 'last, first'
ubezpieczeni: []
)"sv);
		ASSERT_TRUE(value);
		ASSERT_TRUE(value->payer);
		ASSERT_TRUE(value->payer->first_name);
		ASSERT_TRUE(value->payer->last_name);
		value->payer->first_name = std::nullopt;
		value->prepare_for_write();
		auto const expected_output = R"(wersja: 1
płatnik: {}
ubezpieczeni: []
)"sv;
		auto const actual_output = write(*value);
		ASSERT_EQ(actual_output, expected_output);
	}

	TEST_F(partial_parser_impl, only_first_name) {
		auto value = read(R"(wersja: 1
płatnik:
  nazwisko: 'last, first'
ubezpieczeni: []
)"sv);
		ASSERT_TRUE(value);
		ASSERT_TRUE(value->payer);
		ASSERT_TRUE(value->payer->first_name);
		ASSERT_TRUE(value->payer->last_name);
		value->payer->last_name = std::nullopt;
		value->prepare_for_write();
		auto const expected_output = R"(wersja: 1
płatnik: {}
ubezpieczeni: []
)"sv;
		auto const actual_output = write(*value);
		ASSERT_EQ(actual_output, expected_output);
	}
}  // namespace quick_dra::testing
