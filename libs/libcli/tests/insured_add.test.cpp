// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "run.hpp"

namespace quick_dra::builtin::testing::insured_add {
	static constexpr runnable_testcase tests[] = {
	    {
	        .name = "yes no document"sv,
	        .args =
	            "insured add -y --first A --last B --config .quick_dra.yaml"sv,
	        .stderr =
	            R"(usage: qdra insured add [-h] [--config <path>] [-y] [--first <name>] [--last <name>] [--social-id <number>] [--id-card <number>] [--passport <number>] [--title <code>] [--scale <num>/<den>] [--salary <zł>]
qdra insured add: error: at least one of --social-id, --id-card and --passport is required with -y
)"sv,
	        .returncode = 2,
	    },
	    {
	        .name = "bad title"sv,
	        .args =
	            R"(insured add -y --first A --last B --title "010 0 0" --config .quick_dra.yaml)"sv,
	        .stderr =
	            R"(usage: qdra insured add [-h] [--config <path>] [-y] [--first <name>] [--last <name>] [--social-id <number>] [--id-card <number>] [--passport <number>] [--title <code>] [--scale <num>/<den>] [--salary <zł>]
qdra insured add: error: --title: expecting 6 digits in form `#### # #'
)"sv,
	        .returncode = 2,
	    },
	    {
	        .name = "bad scale"sv,
	        .args =
	            "insured add -y --first A --last B --scale 4/5a --config .quick_dra.yaml"sv,
	        .stderr =
	            R"(usage: qdra insured add [-h] [--config <path>] [-y] [--first <name>] [--last <name>] [--social-id <number>] [--id-card <number>] [--passport <number>] [--title <code>] [--scale <num>/<den>] [--salary <zł>]
qdra insured add: error: --scale: expecting two numbers in form `<num>/<den>`, with denominator not equal to zero
)"sv,
	        .returncode = 2,
	    },
	    {
	        .name = "bad scale NaN"sv,
	        .args =
	            "insured add -y --first A --last B --scale 4/0 --config .quick_dra.yaml"sv,
	        .stderr =
	            R"(usage: qdra insured add [-h] [--config <path>] [-y] [--first <name>] [--last <name>] [--social-id <number>] [--id-card <number>] [--passport <number>] [--title <code>] [--scale <num>/<den>] [--salary <zł>]
qdra insured add: error: --scale: expecting two numbers in form `<num>/<den>`, with denominator not equal to zero
)"sv,
	        .returncode = 2,
	    },
	    {
	        .name = "bad salary"sv,
	        .args =
	            R"(insured add -y --first A --last B --salary "1000000 USD" --config .quick_dra.yaml)"sv,
	        .stderr =
	            R"(usage: qdra insured add [-h] [--config <path>] [-y] [--first <name>] [--last <name>] [--social-id <number>] [--id-card <number>] [--passport <number>] [--title <code>] [--scale <num>/<den>] [--salary <zł>]
qdra insured add: error: --salary: expecting a number with 0.01 increment, with optional PLN or zł suffix
)"sv,
	        .returncode = 2,
	    },
	    {
	        .name = "percent scale"sv,
	        .args =
	            R"(insured add --config .quick_dra.yaml -y --first Name --last Surname --id-card AAA000000 --title "0110 0 0" --scale 25%)"sv,
	        .stdout =
	            "\033[0;90mFirst name set to \033[mName\n"
	            "\033[0;90mLast name set to \033[mSurname\n"
	            "\033[0;90mDocument kind set to \033[m1\n"
	            "\033[0;90mDocument set to \033[mAAA000000\n"
	            "\033[0;90mInsurance title set to \033[m0110 0 0\n"
	            "\033[0;90mPart-time scale set to \033[m1/4\n"
	            "\033[0;90mSalary set to \033[mminimal for a given month\n"
	            ""sv,
	        .stderr =
	            R"(Quick-DRA: file .quick_dra.yaml will be created as needed.
)"sv,
	        .writes =
	            new_file{
	                .name = ".quick_dra.yaml"sv,
	                .cmp = ".quick_dra.insured.AAA000000_25%.yaml"sv,
	            },
	    },
	    {
	        .name = "salary amount"sv,
	        .args =
	            R"(insured add --config .quick_dra.yaml -y --first Name --last Surname --id-card AAA000000 --title "0110 0 0" --salary 120000PLN)"sv,
	        .stdout = "\033[0;90mFirst name set to \033[mName\n"
	                  "\033[0;90mLast name set to \033[mSurname\n"
	                  "\033[0;90mDocument kind set to \033[m1\n"
	                  "\033[0;90mDocument set to \033[mAAA000000\n"
	                  "\033[0;90mInsurance title set to \033[m0110 0 0\n"
	                  "\033[0;90mPart-time scale set to \033[m<empty>\n"
	                  "\033[0;90mSalary set to \033[m120000 zł\n"
	                  ""sv,
	        .stderr =
	            R"(Quick-DRA: file .quick_dra.yaml will be created as needed.
)"sv,
	        .writes =
	            new_file{
	                .name = ".quick_dra.yaml"sv,
	                .cmp = ".quick_dra.insured.AAA000000_120k.yaml"sv,
	            },
	    },
	    {
	        .name = "salary minimal"sv,
	        .args =
	            R"(insured add --config .quick_dra.yaml -y --first Name --last Surname --id-card AAA000000 --title "0110 0 0" --scale 3/4 --salary minimal)"sv,
	        .stdout =
	            "\033[0;90mFirst name set to \033[mName\n"
	            "\033[0;90mLast name set to \033[mSurname\n"
	            "\033[0;90mDocument kind set to \033[m1\n"
	            "\033[0;90mDocument set to \033[mAAA000000\n"
	            "\033[0;90mInsurance title set to \033[m0110 0 0\n"
	            "\033[0;90mPart-time scale set to \033[m3/4\n"
	            "\033[0;90mSalary set to \033[mminimal for a given month\n"
	            ""sv,
	        .stderr =
	            R"(Quick-DRA: file .quick_dra.yaml will be created as needed.
)"sv,
	        .writes =
	            new_file{
	                .name = ".quick_dra.yaml"sv,
	                .cmp = ".quick_dra.insured.AAA000000_75%.yaml"sv,
	            },
	    },
	    {
	        .name = "duplicate found"sv,
	        .args =
	            R"(insured add --config .quick_dra.yaml -y --first John --last Hancock --id-card AAA000000 --title "0110 0 0")"sv,
	        .config = R"(wersja: 1
ubezpieczeni:
  - nazwisko: 'Surname, Name'
    dowód: AAA000000
    tytuł ubezpieczenia: 0110 0 0
    wymiar: 3/4
)"sv,
	        .stdout = R"(Found another person with this document:

    #1: Name Surname [1 AAA000000]

Either remove this person or call

    qdra insured edit --pos 1

)"sv,
	        .returncode = 1,
	    },
	    {
	        .name = "full time"sv,
	        .args =
	            R"(insured add --config .quick_dra.yaml -y --first Name --last Surname --id-card AAA000000 --title "0110 0 0" --scale 4/4)"sv,
	        .stdout =
	            "\033[0;90mFirst name set to \033[mName\n"
	            "\033[0;90mLast name set to \033[mSurname\n"
	            "\033[0;90mDocument kind set to \033[m1\n"
	            "\033[0;90mDocument set to \033[mAAA000000\n"
	            "\033[0;90mInsurance title set to \033[m0110 0 0\n"
	            "\033[0;90mPart-time scale set to \033[m1/1\n"
	            "\033[0;90mSalary set to \033[mminimal for a given month\n"
	            ""sv,
	        .stderr =
	            R"(Quick-DRA: file .quick_dra.yaml will be created as needed.
)"sv,
	        .writes =
	            new_file{
	                .name = ".quick_dra.yaml"sv,
	                .cmp = ".quick_dra.insured.AAA000000_full_time.yaml"sv,
	            },
	    },
	    {
	        .name = "config ro"sv,
	        .args =
	            R"(insured add --config .quick_dra.yaml -y --first Name --last Surname --id-card AAA000000 --title "0110 0 0")"sv,
	        .config_name = ".quick_dra.yaml"sv,
	        .stdout =
	            "\033[0;90mFirst name set to \033[mName\n"
	            "\033[0;90mLast name set to \033[mSurname\n"
	            "\033[0;90mDocument kind set to \033[m1\n"
	            "\033[0;90mDocument set to \033[mAAA000000\n"
	            "\033[0;90mInsurance title set to \033[m0110 0 0\n"
	            "\033[0;90mPart-time scale set to \033[m<empty>\n"
	            "\033[0;90mSalary set to \033[mminimal for a given month\n"
	            ""sv,
	        .stderr = R"(Quick-DRA: error: could not write to .quick_dra.yaml
)"sv,
	        .returncode = 1,
	        .mode = readonly_perms,
	    },
	    {
	        .name = "reset all props A"sv,
	        .args =
	            R"(insured add --config .quick_dra.yaml -y --first "" --last "" --social-id "")"sv,
	        .config = R"(wersja: 1
płatnik:
  nazwisko: 'Nowak, Jan'
  dowód: ABC523456
  nip: 7680002466
  pesel: 26211012346
ubezpieczeni: []
)"sv,
	        .stderr =
	            R"(usage: qdra insured add [-h] [--config <path>] [-y] [--first <name>] [--last <name>] [--social-id <number>] [--id-card <number>] [--passport <number>] [--title <code>] [--scale <num>/<den>] [--salary <zł>]
qdra insured add: error: argument --first is required with -y
)"sv,
	        .returncode = 2,
	    },
	    {
	        .name = "reset all props B"sv,
	        .args =
	            R"(insured add --config .quick_dra.yaml -y --first "" --last "" --id-card "")"sv,
	        .config = R"(wersja: 1
płatnik:
  nazwisko: 'Nowak, Jan'
  dowód: ABC523456
  nip: 7680002466
  pesel: 26211012346
ubezpieczeni: []
)"sv,
	        .stderr =
	            R"(usage: qdra insured add [-h] [--config <path>] [-y] [--first <name>] [--last <name>] [--social-id <number>] [--id-card <number>] [--passport <number>] [--title <code>] [--scale <num>/<den>] [--salary <zł>]
qdra insured add: error: argument --first is required with -y
)"sv,
	        .returncode = 2,
	    },
	    {
	        .name = "reset all props C"sv,
	        .args =
	            R"(insured add --config .quick_dra.yaml -y --first "" --last "" --passport "")"sv,
	        .config = R"(wersja: 1
płatnik:
  nazwisko: 'Nowak, Jan'
  dowód: ABC523456
  nip: 7680002466
  pesel: 26211012346
ubezpieczeni: []
)"sv,
	        .stderr =
	            R"(usage: qdra insured add [-h] [--config <path>] [-y] [--first <name>] [--last <name>] [--social-id <number>] [--id-card <number>] [--passport <number>] [--title <code>] [--scale <num>/<den>] [--salary <zł>]
qdra insured add: error: argument --first is required with -y
)"sv,
	        .returncode = 2,
	    },
	};

	INSTANTIATE_TEST_SUITE_P(insured_add, cli_test, ::testing::ValuesIn(tests));
}  // namespace quick_dra::builtin::testing::insured_add
