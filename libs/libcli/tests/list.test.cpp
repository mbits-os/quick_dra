// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "run.hpp"

namespace quick_dra::builtin::testing::list {
	static constexpr runnable_testcase tests[] = {
	    {
	        .name = "empty list"sv,
	        .args = "list --config .quick_dra.yaml"sv,
	    },
	    {
	        .name = "normal out"sv,
	        .args = "list --config .quick_dra.yaml"sv,
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
	        .stdout = R"(#1: Piotr Iksiński [1 ABC523456] 1/4 of <minimal>
#2: Jan Iksiński [2 EH0123456] 3/4 of 9000 zł
#3: Maria Iksińska [P 26211012346] 7500 zł
)"sv,
	    },
	    {
	        .name = "normal out (payer)"sv,
	        .args = "list --config .quick_dra.yaml"sv,
	        .config = R"(wersja: 1
płatnik:
  nazwisko: 'Nowak, Jan'
  paszport: AB4123456
  nip: 7680002466
  pesel: 26211012346
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
	        .stdout =
	            R"(Payer: Jan Nowak [2 AB4123456] PESEL:26211012346 NIP:7680002466
#1: Piotr Iksiński [1 ABC523456] 1/4 of <minimal>
#2: Jan Iksiński [2 EH0123456] 3/4 of 9000 zł
#3: Maria Iksińska [P 26211012346] 7500 zł
)"sv,
	    },
	    {
	        .name = "partial search"sv,
	        .args = "list --config .quick_dra.yaml --find iksiń"sv,
	        .config = R"(wersja: 1
płatnik:
  nazwisko: 'Nowak, Jan'
  paszport: AB4123456
  nip: 7680002466
  pesel: 26211012346
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
	        .stdout =
	            R"(#1: Piotr Iksiński [1 ABC523456] 1/4 of <minimal>
#2: Jan Iksiński [2 EH0123456] 3/4 of 9000 zł
#3: Maria Iksińska [P 26211012346] 7500 zł
)"sv,
	    },
	    {
	        .name = "full search"sv,
	        .args = "list --config .quick_dra.yaml --find maria"sv,
	        .config = R"(wersja: 1
płatnik:
  nazwisko: 'Nowak, Marian'
  paszport: AB4123456
  nip: 7680002466
  pesel: 26211012346
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
	        .stdout =
	            R"(#3: Maria Iksińska [P 26211012346] 7500 zł
)"sv,
	    },
	    {
	        .name = "partial search with payer"sv,
	        .args = "list --config .quick_dra.yaml --find maria"sv,
	        .config = R"(wersja: 1
płatnik:
  nazwisko: 'Nowak, Marian'
  paszport: AB4123456
  nip: 7680002466
  pesel: 26211012346
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
  - nazwisko: 'Iksińska, Marianna'
    tytuł ubezpieczenia: 0110 0 0
    pesel: 26211012346
    pensja: 7500 zł
)"sv,
	        .stdout =
	            R"(Payer: Marian Nowak [2 AB4123456] PESEL:26211012346 NIP:7680002466
#3: Marianna Iksińska [P 26211012346] 7500 zł
)"sv,
	    },
	    {
	        .name = "partial search only payer"sv,
	        .args = "list --config .quick_dra.yaml --find now"sv,
	        .config = R"(wersja: 1
płatnik:
  nazwisko: 'Nowak, Jan'
  paszport: AB4123456
  nip: 7680002466
  pesel: 26211012346
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
	        .stdout =
	            R"(Payer: Jan Nowak [2 AB4123456] PESEL:26211012346 NIP:7680002466
)"sv,
	    },
	    {
	        .name = "empty search"sv,
	        .args = "list --config .quick_dra.yaml --find xzyyz"sv,
	        .config = R"(wersja: 1
płatnik:
  nazwisko: 'Nowak, Jan'
  paszport: AB4123456
  nip: 7680002466
  pesel: 26211012346
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
	        .stdout = ""sv,
	        .stderr =
	            R"(usage: qdra list [-h] [--config <path>] [--find <keyword>] [--pipe] [-z]
qdra list: error: --find: could not find any record using `xzyyz'
)"sv,
	        .returncode = 2,
	    },
	    {
	        .name = "pipe"sv,
	        .args = "list --config .quick_dra.yaml --pipe"sv,
	        .config = R"(wersja: 1
płatnik:
  nazwisko: 'Nowak, Jan'
  paszport: AB4123456
  nip: 7680002466
  pesel: 26211012346
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
	        .stdout =
	            R"(P	Nowak	Jan	2	AB4123456	7680002466	26211012346
1	Iksiński	Piotr	1	ABC523456	0110 0 0	1/4		
2	Iksiński	Jan	2	EH0123456	0110 0 0	3/4	9000 zł	
3	Iksińska	Maria	P	26211012346	0110 0 0		7500 zł	
)"sv,
	    },
	    {
	        .name = "pipe z"sv,
	        .args = "list --config .quick_dra.yaml --pipe -z"sv,
	        .config = R"(wersja: 1
płatnik:
  nazwisko: 'Nowak, Jan'
  paszport: AB4123456
  nip: 7680002466
  pesel: 26211012346
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
	        .stdout = "P\0"
	                  "Nowak\0"
	                  "Jan\0"
	                  "2\0"
	                  "AB4123456\0"
	                  "7680002466\0"
	                  "26211012346\n"
	                  "1\0"
	                  "Iksiński\0"
	                  "Piotr\0"
	                  "1\0"
	                  "ABC523456\0"
	                  "0110 0 0\0"
	                  "1/4\0"
	                  "\0\n"
	                  "2\0"
	                  "Iksiński\0"
	                  "Jan\0"
	                  "2\0"
	                  "EH0123456\0"
	                  "0110 0 0\0"
	                  "3/4\0"
	                  "9000 zł\0\n"
	                  "3\0"
	                  "Iksińska\0"
	                  "Maria\0"
	                  "P\0"
	                  "26211012346\0"
	                  "0110 0 0\0"
	                  "\0"
	                  "7500 zł\0\n"
	                  ""sv,
	    },
	    {
	        .name = "pipe v2"sv,
	        .args = "list --config .quick_dra.yaml --pipe"sv,
	        .config = R"(wersja: 2
płatnik:
  nazwisko: 'Nowak, Jan'
  paszport: AB4123456
  nip: 7680002466
  pesel: 26211012346
ubezpieczeni:
  - nazwisko: 'Iksiński, Piotr'
    dowód: ABC523456
    tytuł ubezpieczenia: 0110 0 0
    historia:
      2022/01:
        wymiar: 1/4
  - nazwisko: 'Iksiński, Jan'
    paszport: EH0123456
    tytuł ubezpieczenia: 0110 0 0
    historia:
      2000/6:
        wymiar: 3/4
        pensja: 9000 zł
  - nazwisko: 'Iksińska, Maria'
    tytuł ubezpieczenia: 0110 0 0
    pesel: 26211012346
    historia:
      2026/05:
        pensja: 7500 zł
)"sv,
	        .stdout =
	            R"(P	Nowak	Jan	2	AB4123456	7680002466	26211012346
1	Iksiński	Piotr	1	ABC523456	0110 0 0	1/4		2022/01
2	Iksiński	Jan	2	EH0123456	0110 0 0	3/4	9000 zł	2000/06
3	Iksińska	Maria	P	26211012346	0110 0 0		7500 zł	2026/05
)"sv,
	    },
	    {
	        .name = "pipe v2, richer history"sv,
	        .args = "list --config .quick_dra.yaml --pipe"sv,
	        .config = R"(wersja: 2
płatnik:
  nazwisko: 'Nowak, Jan'
  paszport: AB4123456
  nip: 7680002466
  pesel: 26211012346
ubezpieczeni:
  - nazwisko: 'Iksiński, Piotr'
    dowód: ABC523456
    tytuł ubezpieczenia: 0110 0 0
    historia:
      2022/01:
        wymiar: 1/4
      2026/01:
        pensja: 8000 zł
  - nazwisko: 'Iksiński, Jan'
    paszport: EH0123456
    tytuł ubezpieczenia: 0110 0 0
    historia:
      2000/6:
        wymiar: 3/4
        pensja: 4000 zł
      2001/6:
        wymiar: 3/4
        pensja: 5000 zł
      2002/6:
        wymiar: 3/4
        pensja: 6000 zł
      2003/6:
        wymiar: 3/4
        pensja: 7000 zł
      2004/6:
        wymiar: 3/4
        pensja: 8000 zł
      2009/6:
        pensja: 9000 zł
  - nazwisko: 'Iksińska, Maria'
    tytuł ubezpieczenia: 0110 0 0
    pesel: 26211012346
    historia:
      2026/05:
        pensja: 7500 zł
)"sv,
	        .stdout =
	            R"(P	Nowak	Jan	2	AB4123456	7680002466	26211012346
1	Iksiński	Piotr	1	ABC523456	0110 0 0		8000 zł	2026/01
2	Iksiński	Jan	2	EH0123456	0110 0 0		9000 zł	2009/06
3	Iksińska	Maria	P	26211012346	0110 0 0		7500 zł	2026/05
)"sv,
	    },
	    {
	        .name = "list v2, richer history"sv,
	        .args = "list --config .quick_dra.yaml"sv,
	        .config = R"(wersja: 2
płatnik:
  nazwisko: 'Nowak, Jan'
  paszport: AB4123456
  nip: 7680002466
  pesel: 26211012346
ubezpieczeni:
  - nazwisko: 'Iksiński, Piotr'
    dowód: ABC523456
    tytuł ubezpieczenia: 0110 0 0
    historia:
      2022/01:
        wymiar: 1/4
      2026/01:
        pensja: 8000 zł
  - nazwisko: 'Iksiński, Jan'
    paszport: EH0123456
    tytuł ubezpieczenia: 0110 0 0
    historia:
      2000/6:
        wymiar: 3/4
        pensja: 4000 zł
      2001/6:
        wymiar: 3/4
        pensja: 5000 zł
      2002/6:
        wymiar: 3/4
        pensja: 6000 zł
      2003/6:
        wymiar: 3/4
        pensja: 7000 zł
      2004/6:
        wymiar: 3/4
        pensja: 8000 zł
      2009/6:
        pensja: 9000 zł
  - nazwisko: 'Iksińska, Maria'
    tytuł ubezpieczenia: 0110 0 0
    pesel: 26211012346
    historia:
      2026/05:
        pensja: 7500 zł
)"sv,
	        .stdout =
	            R"(Payer: Jan Nowak [2 AB4123456] PESEL:26211012346 NIP:7680002466
#1: Piotr Iksiński [1 ABC523456] 8000 zł (since January 2026)
#2: Jan Iksiński [2 EH0123456] 9000 zł (since June 2009)
#3: Maria Iksińska [P 26211012346] 7500 zł (since May 2026)
)"sv,
	    },
	    {
	        .name = "bad config"sv,
	        .args = "list --config .quick_dra.yaml"sv,
	        .config = R"(wersja: 1
płatnik:
  nazwisko: John Smith 2nd
ubezpieczeni:
  - nazwisko: John Smith the Third
)"sv,
	        .stdout =
	            R"(Payer: ?? ?? [?? ??] PESEL:?? NIP:??
#1: ?? ?? [?? ??] <outside employment>
)"sv,
	    },
	    {
	        .name = "bad config in pipe"sv,
	        .args = "list --config .quick_dra.yaml --pipe"sv,
	        .config = R"(wersja: 1
płatnik:
  nazwisko: John Smith 2nd
ubezpieczeni:
  - nazwisko: John Smith the Third
)"sv,
	        .stdout = "P\t\t\t\t\t\t\n"
	                  "1\t\t\t\t\t\t\t\t\n"sv,
	    },
	};

	INSTANTIATE_TEST_SUITE_P(list, cli_test, ::testing::ValuesIn(tests));
}  // namespace quick_dra::builtin::testing::list
