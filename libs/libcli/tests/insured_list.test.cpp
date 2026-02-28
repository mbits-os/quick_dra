// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "run.hpp"

namespace quick_dra::builtin::testing::insured_list {
	static constexpr runnable_testcase tests[] = {
	    {
	        .name = "empty list"sv,
	        .args = "insured list --config .quick_dra.yaml"sv,
	    },
	    {
	        .name = "normal out"sv,
	        .args = "insured list --config .quick_dra.yaml"sv,
	        .stdout = R"(#1: Piotr Iksiński [1 ABC523456] 1/4 of <minimal>
#2: Jan Iksiński [2 EH0123456] 3/4 of 9000 zł
#3: Maria Iksińska [P 26211012346] 7500 zł
)"sv,
	        .config_name = ".quick_dra.yaml"sv,
	        .config = R"(wersja: 1
ubezpieczeni:
  - nazwisko: 'Iksiński, Piotr'
    dowód: ABC523456
    tytuł ubezpieczenia: 0110 0 0
    wymiar: 1/4
  - nazwisko: 'Iksiński, Jan'
    paszport: EH0123456
    tytuł ubezpieczenia: 0110 0 0
    wymiar: 3/4
    pensja: 9000 zł
  - nazwisko: 'Iksińska, Maria'
    tytuł ubezpieczenia: 0110 0 0
    pesel: 26211012346
    pensja: 7500 zł
)"sv,
	    },
	    {
	        .name = "pipe"sv,
	        .args = "insured list --config .quick_dra.yaml --pipe"sv,
	        .stdout = R"(1	Iksiński	Piotr	1	ABC523456	0110 0 0	1/4	
2	Iksiński	Jan	2	EH0123456	0110 0 0	3/4	9000 zł
3	Iksińska	Maria	P	26211012346	0110 0 0		7500 zł
)"sv,
	        .config_name = ".quick_dra.yaml"sv,
	        .config = R"(wersja: 1
ubezpieczeni:
  - nazwisko: 'Iksiński, Piotr'
    dowód: ABC523456
    tytuł ubezpieczenia: 0110 0 0
    wymiar: 1/4
  - nazwisko: 'Iksiński, Jan'
    paszport: EH0123456
    tytuł ubezpieczenia: 0110 0 0
    wymiar: 3/4
    pensja: 9000 zł
  - nazwisko: 'Iksińska, Maria'
    tytuł ubezpieczenia: 0110 0 0
    pesel: 26211012346
    pensja: 7500 zł
)"sv,
	    },
	    {
	        .name = "pipe z"sv,
	        .args = "insured list --config .quick_dra.yaml --pipe -z"sv,
	        .stdout = "1\0"
	                  "Iksiński\0"
	                  "Piotr\0"
	                  "1\0"
	                  "ABC523456\0"
	                  "0110 0 0\0"
	                  "1/4\0"
	                  "\n"
	                  "2\0"
	                  "Iksiński\0"
	                  "Jan\0"
	                  "2\0"
	                  "EH0123456\0"
	                  "0110 0 0\0"
	                  "3/4\0"
	                  "9000 zł\n"
	                  "3\0"
	                  "Iksińska\0"
	                  "Maria\0"
	                  "P\0"
	                  "26211012346\0"
	                  "0110 0 0\0"
	                  "\0"
	                  "7500 zł\n"
	                  ""sv,
	        .config_name = ".quick_dra.yaml"sv,
	        .config = R"(wersja: 1
ubezpieczeni:
  - nazwisko: 'Iksiński, Piotr'
    dowód: ABC523456
    tytuł ubezpieczenia: 0110 0 0
    wymiar: 1/4
  - nazwisko: 'Iksiński, Jan'
    paszport: EH0123456
    tytuł ubezpieczenia: 0110 0 0
    wymiar: 3/4
    pensja: 9000 zł
  - nazwisko: 'Iksińska, Maria'
    tytuł ubezpieczenia: 0110 0 0
    pesel: 26211012346
    pensja: 7500 zł
)"sv,
	    },
	    {
	        .name = "bad config"sv,
	        .args = "insured list --config .quick_dra.yaml"sv,
	        .stdout = R"(#1: ?? ?? [?? ??] <minimal>
)"sv,
	        .config_name = ".quick_dra.yaml"sv,
	        .config = R"(wersja: 1
ubezpieczeni:
  - nazwisko: John Smith the Third
)"sv,
	    },
	};

	INSTANTIATE_TEST_SUITE_P(insured_list,
	                         cli_test,
	                         ::testing::ValuesIn(tests));
}  // namespace quick_dra::builtin::testing::insured_list
