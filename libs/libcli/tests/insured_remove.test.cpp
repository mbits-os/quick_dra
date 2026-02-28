// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "run.hpp"

namespace quick_dra::builtin::testing::insured_remove {
	static constexpr runnable_testcase tests[] = {
	    {
	        .name = "empty list"sv,
	        .args = "insured remove --pos 1 -y --config .quick_dra.yaml"sv,
	        .returncode = 1,
	        .stderr =
	            R"(Quick-DRA: file .quick_dra.yaml will be created as needed.
qdra insured remove: error: there are no items to remove.
)"sv,
	    },
	    {
	        .name = "ambiguous call"sv,
	        .args = "insured remove --find iksiń -y --config .quick_dra.yaml"sv,
	        .returncode = 1,
	        .stdout = R"(Found:
    #1: Piotr Iksiński [1 AAA000000]
    #2: Jan Iksiński [2 AA0000000]

This search was ambiguous. Please refine your parameter or use --pos to pinpoint the record.
)"sv,
	        .config_name = ".quick_dra.yaml"sv,
	        .config = R"(wersja: 1
ubezpieczeni:
  - nazwisko: 'Iksiński, Piotr'
    dowód: AAA000000
    tytuł ubezpieczenia: 0110 0 0
  - nazwisko: 'Iksiński, Jan'
    paszport: AA0000000
    tytuł ubezpieczenia: 0110 0 0
)"sv,
	    },
	    {
	        .name = "remove by pos"sv,
	        .args = "insured remove --pos 2 -y --config .quick_dra.yaml"sv,
	        .stdout = R"(Found:
    #2: Jan Iksiński [2 AA0000000]
)"sv,
	        .config_name = ".quick_dra.yaml"sv,
	        .config = R"(wersja: 1
ubezpieczeni:
  - nazwisko: 'Iksiński, Piotr'
    dowód: AAA000000
    tytuł ubezpieczenia: 0110 0 0
  - nazwisko: 'Iksiński, Jan'
    paszport: AA0000000
    tytuł ubezpieczenia: 0110 0 0
)"sv,
	        .writes =
	            new_file{
	                .name = ".quick_dra.yaml"sv,
	                .cmp = ".quick_dra.iksinski-only-piotr.yaml"sv,
	            },
	    },
	    {
	        .name = "remove by keyword"sv,
	        .args = "insured remove --find piotr -y --config .quick_dra.yaml"sv,
	        .stdout = R"(Found:
    #1: Piotr Iksiński [1 AAA000000]
)"sv,
	        .config_name = ".quick_dra.yaml"sv,
	        .config = R"(wersja: 1
ubezpieczeni:
  - nazwisko: 'Iksiński, Piotr'
    dowód: AAA000000
    tytuł ubezpieczenia: 0110 0 0
  - nazwisko: 'Iksiński, Jan'
    paszport: AA0000000
    tytuł ubezpieczenia: 0110 0 0
)"sv,
	        .writes =
	            new_file{
	                .name = ".quick_dra.yaml"sv,
	                .cmp = ".quick_dra.iksinski-only-jan.yaml"sv,
	            },
	    },
	    {
	        .name = "not found by pos"sv,
	        .args = "insured remove --pos 10 -y --config .quick_dra.yaml"sv,
	        .returncode = 2,
	        .stderr =
	            R"(usage: qdra insured remove [-h] [--config <path>] [--pos <index>] [--find <keyword>] [-y]
qdra insured remove: error: argument --pos must be between 1 and 3, inclusive
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
	        .name = "not found by keyword"sv,
	        .args = "insured remove --find xyzzy -y --config .quick_dra.yaml"sv,
	        .returncode = 2,
	        .stderr =
	            R"(usage: qdra insured remove [-h] [--config <path>] [--pos <index>] [--find <keyword>] [-y]
qdra insured remove: error: --find: could not find any record using `xyzzy'
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
	        .name = "no find"sv,
	        .args = "insured remove -y --config .quick_dra.yaml"sv,
	        .returncode = 2,
	        .stderr =
	            R"(usage: qdra insured remove [-h] [--config <path>] [--pos <index>] [--find <keyword>] [-y]
qdra insured remove: error: one of --pos and --find argument is required
)"sv,
	    },
	    {
	        .name = "find and pos"sv,
	        .args =
	            "insured remove --pos 1 --find something -y --config .quick_dra.yaml"sv,
	        .returncode = 2,
	        .stderr =
	            R"(usage: qdra insured remove [-h] [--config <path>] [--pos <index>] [--find <keyword>] [-y]
qdra insured remove: error: only one of --pos and --find is allowed
)"sv,
	    },
	    {
	        .name = "config ro"sv,
	        .args = "insured remove --pos 2 --config .quick_dra.yaml -y"sv,
	        .returncode = 1,
	        .stdout = R"(Found:
    #2: Jan Iksiński [2 AA0000000]
)"sv,
	        .stderr = R"(Quick-DRA: error: could not write to .quick_dra.yaml
)"sv,
	        .config_name = ".quick_dra.yaml"sv,
	        .config = R"(wersja: 1
ubezpieczeni:
  - nazwisko: 'Iksiński, Piotr'
    dowód: AAA000000
    tytuł ubezpieczenia: 0110 0 0
  - nazwisko: 'Iksiński, Jan'
    paszport: AA0000000
    tytuł ubezpieczenia: 0110 0 0
)"sv,
	        .mode = readonly_perms,
	    },
	};

	INSTANTIATE_TEST_SUITE_P(insured_remove,
	                         cli_test,
	                         ::testing::ValuesIn(tests));
}  // namespace quick_dra::builtin::testing::insured_remove
