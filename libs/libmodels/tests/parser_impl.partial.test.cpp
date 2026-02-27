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
  dowód: AAA000000
ubezpieczeni: []
)"sv);
		ASSERT_TRUE(value);
		ASSERT_TRUE(value->payer);
		ASSERT_TRUE(value->payer->first_name);
		ASSERT_TRUE(value->payer->last_name);
		ASSERT_TRUE(value->payer->kind);
		ASSERT_TRUE(value->payer->document);
		value->payer->first_name = std::nullopt;
		value->prepare_for_write();
		auto const expected_output = R"(wersja: 1
płatnik:
  dowód: AAA000000
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

	TEST_F(partial_parser_impl, with_insured) {
		auto value = read(R"(wersja: 1
płatnik: {}
ubezpieczeni:
  - nazwisko: 'single name'
    tytuł ubezpieczenia: 1111 2 3
    dowód: AAA000000
)"sv);
		ASSERT_TRUE(value);
		ASSERT_TRUE(value->insured);
		ASSERT_EQ(value->insured->size(), 1);
		ASSERT_FALSE(value->insured->back().first_name);
		ASSERT_FALSE(value->insured->back().last_name);
		ASSERT_TRUE(value->insured->back().title);
		value->insured->emplace_back();
		auto& person = value->insured->back();
		person.first_name = "Name"s;
		person.last_name = "Surname"s;
		person.kind = "2"s;
		person.document = "AA0000000"s;
		person.part_time_scale = ratio{1, 2};
		person.salary = 3'000_PLN;
		value->prepare_for_write();
		auto const expected_output = R"(wersja: 1
płatnik: {}
ubezpieczeni:
  - dowód: AAA000000
    tytuł ubezpieczenia: 1111 2 3
  - nazwisko: 'Surname, Name'
    paszport: AA0000000
    wymiar: 1/2
    pensja: 3000 zł
)"sv;
		auto const actual_output = write(*value);
		ASSERT_EQ(actual_output, expected_output);
	}
}  // namespace quick_dra::testing
