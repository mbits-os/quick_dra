// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "run.hpp"

namespace quick_dra::builtin::testing::insured_edit {
	static constexpr runnable_testcase tests[] = {
	    {
	        .name = "edit middle"sv,
	        .args =
	            "insured edit --pos 2 -y --config .quick_dra.yaml --first Antoni --last Kowalski --social-id 78070707132 --scale 1/1 --salary 6500PLN"sv,
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
	        .stdout = "Found:\n"
	                  "    #2: Jan Iksiński [2 EH0123456]\n"
	                  "\033[0;90mFirst name changed from \033[mJan\033[0;90m "
	                  "to \033[mAntoni\n"
	                  "\033[0;90mLast name changed from "
	                  "\033[mIksiński\033[0;90m to \033[mKowalski\n"
	                  "\033[0;90mDocument kind changed from \033[m2\033[0;90m "
	                  "to \033[mP\n"
	                  "\033[0;90mDocument changed from "
	                  "\033[mEH0123456\033[0;90m to \033[m78070707132\n"
	                  "\033[0;90mPart-time scale changed from "
	                  "\033[m3/4\033[0;90m to \033[m1/1\n"
	                  "\033[0;90mSalary changed from \033[m9000 zł\033[0;90m "
	                  "to \033[m6500 zł\n"
	                  ""sv,
	        .writes =
	            new_file{
	                .name = ".quick_dra.yaml"sv,
	                .cmp = ".quick_dra.001-edit-middle.yaml"sv,
	            },
	    },
	    {
	        .name = "edit by search"sv,
	        .args =
	            "insured edit --find EH0123456 -y --config .quick_dra.yaml --first Antoni --last Kowalski --social-id 78070707132 --scale 1/1 --salary 6500PLN"sv,
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
	        .stdout = "Found:\n"
	                  "    #2: Jan Iksiński [2 EH0123456]\n"
	                  "\033[0;90mFirst name changed from \033[mJan\033[0;90m "
	                  "to \033[mAntoni\n"
	                  "\033[0;90mLast name changed from "
	                  "\033[mIksiński\033[0;90m to \033[mKowalski\n"
	                  "\033[0;90mDocument kind changed from \033[m2\033[0;90m "
	                  "to \033[mP\n"
	                  "\033[0;90mDocument changed from "
	                  "\033[mEH0123456\033[0;90m to \033[m78070707132\n"
	                  "\033[0;90mPart-time scale changed from "
	                  "\033[m3/4\033[0;90m to \033[m1/1\n"
	                  "\033[0;90mSalary changed from \033[m9000 zł\033[0;90m "
	                  "to \033[m6500 zł\n"
	                  ""sv,
	        .writes =
	            new_file{
	                .name = ".quick_dra.yaml"sv,
	                .cmp = ".quick_dra.001-edit-middle.yaml"sv,
	            },
	    },
	    {
	        .name = "no find"sv,
	        .args =
	            "insured edit -y --config .quick_dra.yaml --first Antoni --last Kowalski --social-id 78070707132 --scale 1/1 --salary 6500PLN"sv,
	        .stderr =
	            R"(usage: qdra insured edit [-h] [--config <path>] [--pos <index>] [--find <keyword>] [-y] [--first <name>] [--last <name>] [--social-id <number>] [--id-card <number>] [--passport <number>] [--title <code>] [--scale <num>/<den>] [--salary <zł>]
qdra insured edit: error: one of --pos and --find argument is required
)"sv,
	        .returncode = 2,
	    },
	    {
	        .name = "find and pos"sv,
	        .args =
	            "insured edit --pos 1 --find something -y --config .quick_dra.yaml --first Antoni --last Kowalski --social-id 78070707132 --scale 1/1 --salary 6500PLN"sv,
	        .stderr =
	            R"(usage: qdra insured edit [-h] [--config <path>] [--pos <index>] [--find <keyword>] [-y] [--first <name>] [--last <name>] [--social-id <number>] [--id-card <number>] [--passport <number>] [--title <code>] [--scale <num>/<den>] [--salary <zł>]
qdra insured edit: error: only one of --pos and --find is allowed
)"sv,
	        .returncode = 2,
	    },
	    {
	        .name = "no salary"sv,
	        .args =
	            "insured edit --find EH0123456 -y --config .quick_dra.yaml --first Antoni --last Kowalski --social-id 78070707132 --scale 1/1"sv,
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
	        .stdout = "Found:\n"
	                  "    #2: Jan Iksiński [2 EH0123456]\n"
	                  "\033[0;90mFirst name changed from \033[mJan\033[0;90m "
	                  "to \033[mAntoni\n"
	                  "\033[0;90mLast name changed from "
	                  "\033[mIksiński\033[0;90m to \033[mKowalski\n"
	                  "\033[0;90mDocument kind changed from \033[m2\033[0;90m "
	                  "to \033[mP\n"
	                  "\033[0;90mDocument changed from "
	                  "\033[mEH0123456\033[0;90m to \033[m78070707132\n"
	                  "\033[0;90mPart-time scale changed from "
	                  "\033[m3/4\033[0;90m to \033[m1/1\n"
	                  "\033[0;90mSalary changed from \033[m9000 zł\033[0;90m "
	                  "to \033[mminimal for a given month\n"
	                  ""sv,
	        .writes =
	            new_file{
	                .name = ".quick_dra.yaml"sv,
	                .cmp = ".quick_dra.001-edit-middle-no-salary.yaml"sv,
	            },
	    },
	    {
	        .name = "search in empty"sv,
	        .args = "insured edit --find EH0123456 --config .quick_dra.yaml"sv,
	        .stderr =
	            R"(Quick-DRA: file .quick_dra.yaml will be created as needed.
usage: qdra insured edit [-h] [--config <path>] [--pos <index>] [--find <keyword>] [-y] [--first <name>] [--last <name>] [--social-id <number>] [--id-card <number>] [--passport <number>] [--title <code>] [--scale <num>/<den>] [--salary <zł>]
qdra insured edit: error: --find: could not find any record using `EH0123456'
)"sv,
	        .returncode = 2,
	    },
	    {
	        .name = "ambiguous"sv,
	        .args = "insured edit --find Iksiń -y --config .quick_dra.yaml"sv,
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
	        .stdout = R"(Found:
    #1: Piotr Iksiński [1 ABC523456]
    #2: Jan Iksiński [2 EH0123456]
    #3: Maria Iksińska [P 26211012346]

This search was ambiguous. Please refine your parameter or use --pos to pinpoint the record.
)"sv,
	        .returncode = 1,
	    },
	    {
	        .name = "config ro"sv,
	        .args =
	            "insured edit --find Maria -y --config .quick_dra.yaml --first Antoni --last Kowalski --social-id 78070707132 --scale 1/1 --salary 6500PLN"sv,
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
	        .stdout = "Found:\n"
	                  "    #3: Maria Iksińska [P 26211012346]\n"
	                  "\033[0;90mFirst name changed from \033[mMaria\033[0;90m "
	                  "to \033[mAntoni\n"
	                  "\033[0;90mLast name changed from "
	                  "\033[mIksińska\033[0;90m to \033[mKowalski\n"
	                  "\033[0;90mDocument changed from "
	                  "\033[m26211012346\033[0;90m to \033[m78070707132\n"
	                  "\033[0;90mSalary changed from \033[m7500 zł\033[0;90m "
	                  "to \033[m6500 zł\n"
	                  ""sv,
	        .stderr = R"(Quick-DRA: error: could not write to .quick_dra.yaml
)"sv,
	        .returncode = 1,
	        .mode = readonly_perms,
	    },
	    {
	        .name = "conflict"sv,
	        .args =
	            "insured edit --find Maria -y --config .quick_dra.yaml --first Antoni --last Kowalski --passport EH0123456 --scale 1/1 --salary 6500PLN"sv,
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
	        .stdout = "Found:\n"
	                  "    #3: Maria Iksińska [P 26211012346]\n"
	                  "Found another person with this document:\n"
	                  "\n"
	                  "    #1: Jan Iksiński [2 EH0123456]\n"
	                  "\n"
	                  "\033[0;90m - The passport number provided seems to be "
	                  "invalid.\033[m\n"
	                  "\033[0;90m - Cannot save invalid data with -y. "
	                  "Stopping.\033[m\n"
	                  ""sv,
	        .returncode = 1,
	    },
	    {
	        .name = "reset all props A"sv,
	        .args =
	            R"(insured edit --find Maria --config .quick_dra.yaml -y --first "" --last "" --social-id "")"sv,
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
	        .stderr =
	            R"(usage: qdra insured edit [-h] [--config <path>] [--pos <index>] [--find <keyword>] [-y] [--first <name>] [--last <name>] [--social-id <number>] [--id-card <number>] [--passport <number>] [--title <code>] [--scale <num>/<den>] [--salary <zł>]
qdra insured edit: error: at least one of --social-id, --id-card and --passport is required with -y
)"sv,
	        .returncode = 2,
	    },
	    {
	        .name = "reset all props B"sv,
	        .args =
	            R"(insured edit --find Maria --config .quick_dra.yaml -y --first "" --last "" --id-card "")"sv,
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
	        .stderr =
	            R"(usage: qdra insured edit [-h] [--config <path>] [--pos <index>] [--find <keyword>] [-y] [--first <name>] [--last <name>] [--social-id <number>] [--id-card <number>] [--passport <number>] [--title <code>] [--scale <num>/<den>] [--salary <zł>]
qdra insured edit: error: at least one of --social-id, --id-card and --passport is required with -y
)"sv,
	        .returncode = 2,
	    },
	    {
	        .name = "reset all props C"sv,
	        .args =
	            R"(insured edit --find Maria --config .quick_dra.yaml -y --first "" --last "" --passport "")"sv,
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
	        .stderr =
	            R"(usage: qdra insured edit [-h] [--config <path>] [--pos <index>] [--find <keyword>] [-y] [--first <name>] [--last <name>] [--social-id <number>] [--id-card <number>] [--passport <number>] [--title <code>] [--scale <num>/<den>] [--salary <zł>]
qdra insured edit: error: at least one of --social-id, --id-card and --passport is required with -y
)"sv,
	        .returncode = 2,
	    },
	};

	INSTANTIATE_TEST_SUITE_P(insured_edit,
	                         cli_test,
	                         ::testing::ValuesIn(tests));
}  // namespace quick_dra::builtin::testing::insured_edit
