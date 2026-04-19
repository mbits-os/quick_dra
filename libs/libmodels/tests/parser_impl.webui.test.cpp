// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "parser_impl.common.hpp"

namespace quick_dra::testing {
	struct partial_reader : object_reader<partial::config> {
		std::optional<partial::config> read_partial(std::string_view text) { return read(text); }
	};

	struct webapp_reader : object_reader<webapp::config> {
		std::optional<webapp::config> read_webapp(std::string_view text) { return read(text); }
	};

	class webui_parser_impl : public ::testing::Test, public partial_reader, public webapp_reader {
	public:
		using partial_reader::write;
		using webapp_reader::write;
	};

	TEST_F(webui_parser_impl, take_from_empty) {
		auto value = read_partial(R"(płatnik: {})"sv);
		ASSERT_TRUE(value);

		webapp::config cfg{};
		take_from(cfg, *value);

		auto const actual_output = cfg.store();
		auto const expected_output = R"({"payer": {},"insured": []})"sv;
		ASSERT_EQ(actual_output, expected_output);
	}

	TEST_F(webui_parser_impl, take_from_name_id) {
		auto value = read_partial(R"(wersja: 1
płatnik:
  nazwisko: 'last, first'
  dowód: AAA000000
ubezpieczeni: []
)"sv);
		ASSERT_TRUE(value);

		webapp::config cfg{};
		take_from(cfg, *value);

		auto const actual_output = cfg.store();
		auto const expected_output = R"({"payer": )"
		                             R"({"first_name": "first","last_name": "last","kind": 1,"document": "AAA000000"},)"
		                             R"("insured": []})"sv;
		ASSERT_EQ(actual_output, expected_output);
	}

	TEST_F(webui_parser_impl, take_from_multiple_insured) {
		auto value = read_partial(R"(wersja: 1
płatnik: {}
ubezpieczeni:
  - dowód: AAA000000
    tytuł ubezpieczenia: 1111 2 3
  - nazwisko: 'Surname, Name'
    paszport: AA0000000
    wymiar: 1/2
    pensja: 3000 zł
)"sv);
		ASSERT_TRUE(value);

		webapp::config cfg{};
		take_from(cfg, *value);

		auto const actual_output = cfg.store();
		auto const expected_output =
		    R"({"payer": {},)"
		    R"("insured": [)"
		    R"({"kind": 1,"document": "AAA000000","title": {"title_code": 1111,"pension_right": 2,"disability_level": 3}},)"
		    R"({"first_name": "Name","last_name": "Surname","kind": 2,"document": "AA0000000","part_time_scale": {"num": 1,"den": 2},"salary": "3000 zł"})"
		    R"(]})"sv;
		ASSERT_EQ(actual_output, expected_output);
	}
}  // namespace quick_dra::testing
