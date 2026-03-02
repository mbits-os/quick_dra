// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "parser_impl.common.hpp"

namespace quick_dra::testing {

	struct payer_view {
		std::string_view last_name;
		std::string_view first_name;
		std::string_view kind;
		std::string_view document;
		std::string_view tax_id;
		std::string_view social_id;
		std::optional<std::string_view> id_card;
		std::optional<std::string_view> passport;

		[[nodiscard]] payer_t to_payer() const {
			return {{
			            .last_name = as_str(last_name),
			            .id_card = as_str(id_card),
			            .passport = as_str(passport),
			            .first_name = as_str(first_name),
			            .kind = as_str(kind),
			            .document = as_str(document),
			        },
			        as_str(tax_id),
			        as_str(social_id)};
		}
	};

	struct insured_view {
		std::string_view last_name;
		std::string_view first_name;
		std::string_view kind;
		std::string_view document;
		std::string_view title;
		std::optional<ratio> part_time_scale;
		std::optional<currency> salary;
		std::optional<std::string_view> id_card;
		std::optional<std::string_view> passport;
		std::optional<std::string_view> social_id;

		[[nodiscard]] insured_t to_insured() const {
			insurance_title out_title;
			insurance_title::parse(title, out_title);
			return {{
			            .last_name = as_str(last_name),
			            .id_card = as_str(id_card),
			            .passport = as_str(passport),
			            .first_name = as_str(first_name),
			            .kind = as_str(kind),
			            .document = as_str(document),
			        },
			        out_title,
			        as_str(social_id),
			        part_time_scale,
			        salary};
		}
	};

	struct config_view {
		unsigned short version;
		payer_view payer;
		std::span<insured_view const> insured;

		[[nodiscard]] config to_config() const {
			std::vector<insured_t> out_insured;
			out_insured.reserve(insured.size());
			std::transform(
			    insured.begin(), insured.end(), std::back_inserter(out_insured),
			    [](auto const& person) { return person.to_insured(); });
			return {
			    .version = version,
			    .payer = payer.to_payer(),
			    .insured = std::move(out_insured),
			    .params{},
			};
		}
	};

	class bad_parser_impl : public ::testing::TestWithParam<
	                            std::pair<std::string_view, std::string_view>>,
	                        public object_reader<config> {};

	TEST_P(bad_parser_impl, load) {
		auto const& [yaml, expected_log] = GetParam();
		auto const value = read(yaml);
		ASSERT_FALSE(value);
		ASSERT_EQ(log, expected_log);
	}

	class good_parser_impl : public ::testing::TestWithParam<
	                             std::pair<std::string_view, config_view>>,
	                         public object_reader<config> {};

	TEST_P(good_parser_impl, load) {
		auto const& [yaml, expected_config] = GetParam();
		auto const opt = read(yaml);
		ASSERT_TRUE(opt);
		auto const& value = *opt;
		auto const expected = expected_config.to_config();
		ASSERT_EQ(value.version, expected.version);
		ASSERT_EQ(value.payer.last_name, expected.payer.last_name);
		ASSERT_EQ(value.payer.id_card, expected.payer.id_card);
		ASSERT_EQ(value.payer.passport, expected.payer.passport);
		ASSERT_EQ(value.payer.first_name, expected.payer.first_name);
		ASSERT_EQ(value.payer.kind, expected.payer.kind);
		ASSERT_EQ(value.payer.document, expected.payer.document);
		ASSERT_EQ(value.payer, expected.payer);
		ASSERT_EQ(value.insured, expected.insured);
	}

	static constexpr std::pair<std::string_view, std::string_view>
	    bad_configs[] = {
	        {
	            ""sv,
	            R"(error: expecting `wersja`; please use explicit {} to heave empty object instead
error: while reading `wersja`
)"sv,
	        },
	        {
	            R"(wersja: 1
)"sv,
	            R"(input:1:1: error: expecting `płatnik`
input:1:1: error: while reading `płatnik`
)"sv,
	        },
	        {
	            R"(wersja: 1
płatnik: {}
)"sv,
	            R"(input:2:1: error: expecting `nazwisko`
input:2:1: error: while reading `nazwisko`
input:1:1: error: while reading `płatnik`
)"sv,
	        },
	        {
	            R"(wersja: 1
płatnik:
  nazwisko: 'A, B'
  dowód: AAA000000
  nip: 1234563218
  pesel: 26211012346
)"sv,
	            R"(input:1:1: error: expecting `ubezpieczeni`
input:1:1: error: while reading `ubezpieczeni`
)"sv,
	        },
	        {
	            R"(wersja: 1
płatnik:
  nazwisko: 'A, B'
  dowód: AAA000000
  paszport: AA0000000
  nip: 1234563218
  pesel: 26211012346
ubezpieczeni: []
)"sv,
	            "input:1:1: error: while reading `płatnik`\n"sv,
	        },
	        {
	            R"(wersja: 1
płatnik:
  nazwisko: 'A, B'
  nip: 1234563218
  pesel: 26211012346
ubezpieczeni: []
)"sv,
	            "input:1:1: error: while reading `płatnik`\n"sv,
	        },
	        {
	            R"(wersja: 1
płatnik:
  nazwisko: ', B'
  paszport: AA0000000
  nip: 1234563218
  pesel: 26211012346
ubezpieczeni: []
)"sv,
	            "input:1:1: error: while reading `płatnik`\n"sv,
	        },
	        {
	            R"(wersja: 1
płatnik:
  nazwisko: 'A B'
  paszport: AA0000000
  nip: 1234563218
  pesel: 26211012346
ubezpieczeni: []
)"sv,
	            "input:1:1: error: while reading `płatnik`\n"sv,
	        },
	        {
	            R"(wersja: 1
płatnik:
  nazwisko: 'A, B'
  paszport: AA0000000
  nip: 1234563218
  pesel: 26211012346
ubezpieczeni:
- nazwisko: 'C D'
  dowód: AAA000000
  tytuł ubezpieczenia: 9999 9 9
)"sv,
	            "input:1:1: error: while reading `ubezpieczeni`\n"sv,
	        },
	        {
	            R"(wersja: 1
płatnik:
  nazwisko: 'A, B'
  paszport: AA0000000
  nip: 1234563218
  pesel: 26211012346
ubezpieczeni:
- nazwisko: 'C D'
  pesel: 26211012346
  tytuł ubezpieczenia: 9999 9 9
)"sv,
	            "input:1:1: error: while reading `ubezpieczeni`\n"sv,
	        },
	        {
	            R"(wersja: 1
płatnik:
  nazwisko: 'A, B'
  paszport: AA0000000
  nip: 1234563218
  pesel: 26211012346
ubezpieczeni:
- nazwisko: 'C D'
  dowód: AAA000000
  tytuł ubezpieczenia: 9999 9 9
)"sv,
	            "input:1:1: error: while reading `ubezpieczeni`\n"sv,
	        },
	        {
	            R"(wersja: 1
płatnik:
  nazwisko: 'A, B'
  paszport: AA0000000
  nip: 1234563218
  pesel: 26211012346
ubezpieczeni:
- nazwisko: 'C, D'
  dowód: AAA000000
  paszport: AA0000000
  tytuł ubezpieczenia: 9999 9 9
)"sv,
	            "input:1:1: error: while reading `ubezpieczeni`\n"sv,
	        },
	        {
	            R"(wersja: 1
płatnik:
  nazwisko: 'A, B'
  paszport: AA0000000
  nip: 1234563218
  pesel: 26211012346
ubezpieczeni:
- nazwisko: 'C, D'
  dowód: AAA000000
  paszport: AA0000000
  tytuł ubezpieczenia: 9999 9 9
)"sv,
	            "input:1:1: error: while reading `ubezpieczeni`\n"sv,
	        },
	        {
	            R"(wersja: 1
płatnik:
  nazwisko: 'A, B'
  paszport: AA0000000
  nip: 1234563218
  pesel: 26211012346
ubezpieczeni:
- nazwisko: 'C, D'
  pesel: 26211012346
  paszport: AA0000000
  tytuł ubezpieczenia: 9999 9 9
)"sv,
	            "input:1:1: error: while reading `ubezpieczeni`\n"sv,
	        },
	};

	static constexpr auto only_C_D_id_card = std::array{
	    insured_view{
	        .last_name = "C"sv,
	        .first_name = "D"sv,
	        .kind = "1"sv,
	        .document = "AAA000000"sv,
	        .title = "9999 9 9"sv,
	        .part_time_scale = std::nullopt,
	        .salary = std::nullopt,
	        .id_card = std::nullopt,
	        .passport = std::nullopt,
	        .social_id = std::nullopt,
	    },
	};

	static constexpr auto only_C_D_passport = std::array{
	    insured_view{
	        .last_name = "C"sv,
	        .first_name = "D"sv,
	        .kind = "2"sv,
	        .document = "AA0000000"sv,
	        .title = "9999 9 9"sv,
	        .part_time_scale = std::nullopt,
	        .salary = std::nullopt,
	        .id_card = std::nullopt,
	        .passport = std::nullopt,
	        .social_id = std::nullopt,
	    },
	};

	static constexpr auto only_C_D_social_id = std::array{
	    insured_view{
	        .last_name = "C"sv,
	        .first_name = "D"sv,
	        .kind = "P"sv,
	        .document = "26211012346"sv,
	        .title = "9999 9 9"sv,
	        .part_time_scale = std::nullopt,
	        .salary = std::nullopt,
	        .id_card = std::nullopt,
	        .passport = std::nullopt,
	        .social_id = std::nullopt,
	    },
	};

	static constexpr std::pair<std::string_view, config_view> good_configs[] = {
	    {
	        R"(wersja: 1
płatnik:
  nazwisko: 'A, B'
  paszport: AA0000000
  nip: 1234563218
  pesel: 26211012346
ubezpieczeni:
- nazwisko: 'C, D'
  dowód: AAA000000
  tytuł ubezpieczenia: 9999 9 9
)",
	        config_view{
	            .version = kVersion,
	            .payer =
	                {
	                    .last_name = "A"sv,
	                    .first_name = "B"sv,
	                    .kind = "2"sv,
	                    .document = "AA0000000"sv,
	                    .tax_id = "1234563218"sv,
	                    .social_id = "26211012346"sv,
	                    .id_card = std::nullopt,
	                    .passport = std::nullopt,
	                },
	            .insured = only_C_D_id_card,
	        },
	    },
	    {
	        R"(wersja: 1
płatnik:
  nazwisko: 'A, B'
  dowód: AAA000000
  nip: 1234563218
  pesel: 26211012346
ubezpieczeni:
- nazwisko: 'C, D'
  paszport: AA0000000
  tytuł ubezpieczenia: 9999 9 9
)",
	        config_view{
	            .version = kVersion,
	            .payer =
	                {
	                    .last_name = "A"sv,
	                    .first_name = "B"sv,
	                    .kind = "1"sv,
	                    .document = "AAA000000"sv,
	                    .tax_id = "1234563218"sv,
	                    .social_id = "26211012346"sv,
	                    .id_card = std::nullopt,
	                    .passport = std::nullopt,
	                },
	            .insured = only_C_D_passport,
	        },
	    },
	    {
	        R"(wersja: 1
płatnik:
  nazwisko: 'A, B'
  dowód: AAA000000
  nip: 1234563218
  pesel: 26211012346
ubezpieczeni:
- nazwisko: 'C, D'
  pesel: 26211012346
  tytuł ubezpieczenia: 9999 9 9
)",
	        config_view{
	            .version = kVersion,
	            .payer =
	                {
	                    .last_name = "A"sv,
	                    .first_name = "B"sv,
	                    .kind = "1"sv,
	                    .document = "AAA000000"sv,
	                    .tax_id = "1234563218"sv,
	                    .social_id = "26211012346"sv,
	                    .id_card = std::nullopt,
	                    .passport = std::nullopt,
	                },
	            .insured = only_C_D_social_id,
	        },
	    },
	};

	INSTANTIATE_TEST_SUITE_P(yaml,
	                         bad_parser_impl,
	                         ::testing::ValuesIn(bad_configs));

	INSTANTIATE_TEST_SUITE_P(yaml,
	                         good_parser_impl,
	                         ::testing::ValuesIn(good_configs));
}  // namespace quick_dra::testing
