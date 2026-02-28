// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "run.hpp"

namespace quick_dra::builtin::testing::payer {
	static constexpr runnable_testcase tests[] = {
	    {
	        .name = "create new id card"sv,
	        .args =
	            "payer --config .quick_dra.yaml -y --first Jan --last Nowak --social-id 26211012346 --tax-id 7680002466 --id-card ABC523456"sv,
	        .stdout = "\033[0;90mFirst name changed from "
	                  "\033[m<empty>\033[0;90m to \033[mJan\n"
	                  "\033[0;90mLast name changed from "
	                  "\033[m<empty>\033[0;90m to \033[mNowak\n"
	                  "\033[0;90mNIP changed from \033[m<empty>\033[0;90m to "
	                  "\033[m7680002466\n"
	                  "\033[0;90mPESEL changed from \033[m<empty>\033[0;90m to "
	                  "\033[m26211012346\n"
	                  "\033[0;90mDocument kind changed from "
	                  "\033[m<empty>\033[0;90m to \033[m1\n"
	                  "\033[0;90mDocument changed from \033[m<empty>\033[0;90m "
	                  "to \033[mABC523456\n"
	                  ""sv,
	        .stderr =
	            R"(Quick-DRA: file .quick_dra.yaml will be created as needed.
)"sv,
	        .writes =
	            new_file{
	                .name = ".quick_dra.yaml"sv,
	                .cmp = ".quick_dra.ABC523456.yaml"sv,
	            },
	    },
	    {
	        .name = "create new passport"sv,
	        .args = "payer --config .quick_dra.yaml -y --passport AB4123456"sv,
	        .stdout = "\033[0;90mDocument kind changed from \033[m1\033[0;90m "
	                  "to \033[m2\n"
	                  "\033[0;90mDocument changed from "
	                  "\033[mABC523456\033[0;90m to \033[mAB4123456\n"
	                  ""sv,
	        .config_name = ".quick_dra.yaml"sv,
	        .config = R"(wersja: 1
płatnik:
  nazwisko: 'Nowak, Jan'
  dowód: ABC523456
  nip: 7680002466
  pesel: 26211012346
ubezpieczeni: []
)"sv,
	        .writes =
	            new_file{
	                .name = ".quick_dra.yaml"sv,
	                .cmp = ".quick_dra.AB4123456.yaml"sv,
	            },
	    },
	    {
	        .name = "create new ro"sv,
	        .args =
	            "payer --config .quick_dra.yaml -y --first Jan --last Nowak --social-id 26211012346 --tax-id 7680002466 --id-card ABC523456"sv,
	        .returncode = 1,
	        .stdout = "\033[0;90mFirst name changed from "
	                  "\033[m<empty>\033[0;90m to \033[mJan\n"
	                  "\033[0;90mLast name changed from "
	                  "\033[m<empty>\033[0;90m to \033[mNowak\n"
	                  "\033[0;90mNIP changed from \033[m<empty>\033[0;90m to "
	                  "\033[m7680002466\n"
	                  "\033[0;90mPESEL changed from \033[m<empty>\033[0;90m to "
	                  "\033[m26211012346\n"
	                  "\033[0;90mDocument kind changed from "
	                  "\033[m<empty>\033[0;90m to \033[m1\n"
	                  "\033[0;90mDocument changed from \033[m<empty>\033[0;90m "
	                  "to \033[mABC523456\n"
	                  ""sv,
	        .stderr = R"(Quick-DRA: error: could not write to .quick_dra.yaml
)"sv,
	        .config_name = ".quick_dra.yaml"sv,
	        .mode = readonly_perms,
	    },
	    {
	        .name = "create new no doc"sv,
	        .args =
	            "payer --config .quick_dra.yaml -y --first Jan --last Nowak --social-id 26211012346 --tax-id 7680002466"sv,
	        .returncode = 2,
	        .stderr =
	            R"(Quick-DRA: file .quick_dra.yaml will be created as needed.
usage: qdra payer [-h] [--config <path>] [-y] [--first <name>] [--last <name>] [--social-id <number>] [--tax-id <number>] [--id-card <number>] [--passport <number>]
qdra payer: error: at least one of --id-card and --passport is required with -y
)"sv,
	    },
	    {
	        .name = "bad passport"sv,
	        .args = "payer --config .quick_dra.yaml -y --passport AB0123456"sv,
	        .returncode = 1,
	        .stdout = "\033[0;90m - The passport number provided seems to be "
	                  "invalid.\033[m\n"
	                  "\033[0;90m - Cannot save invalid data with -y. "
	                  "Stopping.\033[m\n"
	                  ""sv,
	        .config_name = ".quick_dra.yaml"sv,
	        .config = R"(wersja: 1
płatnik:
  nazwisko: 'Nowak, Jan'
  dowód: ABC523456
  nip: 7680002466
  pesel: 26211012346
ubezpieczeni: []
)"sv,
	    },
	    {
	        .name = "reset all props A"sv,
	        .args =
	            "payer --config .quick_dra.yaml -y --first \"\" --last \"\" --social-id \"\" --tax-id \"\" --id-card \"\""sv,
	        .config_name = ".quick_dra.yaml"sv,
	        .config = R"(wersja: 1
płatnik:
  nazwisko: 'Nowak, Jan'
  dowód: ABC523456
  nip: 7680002466
  pesel: 26211012346
ubezpieczeni: []
)"sv,
	    },
	    {
	        .name = "reset all props B"sv,
	        .args =
	            "payer --config .quick_dra.yaml -y --first \"\" --last \"\" --social-id \"\" --tax-id \"\" --passport \"\""sv,
	        .config_name = ".quick_dra.yaml"sv,
	        .config = R"(wersja: 1
płatnik:
  nazwisko: 'Nowak, Jan'
  dowód: ABC523456
  nip: 7680002466
  pesel: 26211012346
ubezpieczeni: []
)"sv,
	    },
	    {
	        .name = "bad config"sv,
	        .args =
	            "payer --config .quick_dra.yaml -y --first Jan --last Nowak --social-id 26211012346 --tax-id 7680002466 --id-card ABC523456"sv,
	        .returncode = 1,
	        .stderr = R"(.quick_dra.yaml:1:1: error: expecting a positive number
.quick_dra.yaml:1:1: error: while reading `wersja`
Quick-DRA: error: .quick_dra.yaml needs to be updated before continuing
)"sv,
	        .config_name = ".quick_dra.yaml"sv,
	        .config = "wersja: John"sv,
	    },
	};

	INSTANTIATE_TEST_SUITE_P(payer, cli_test, ::testing::ValuesIn(tests));
}  // namespace quick_dra::builtin::testing::payer
