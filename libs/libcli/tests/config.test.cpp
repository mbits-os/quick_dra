// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <quick_dra/version.hpp>
#include "run.hpp"

namespace quick_dra::builtin::testing {
	static constexpr runnable_testcase tests[] = {
	    // upgrade
	    {
	        .name = "upgrade empty config"sv,
	        .args = "config upgrade --config .quick_dra.yaml"sv,
	        .config = "wersja: 1"sv,
	        .stdout = "\033[0;90mConfig version updated\033[m\n"sv,
	        .writes =
	            new_file{
	                .name = ".quick_dra.yaml"sv,
	                .cmp = ".config-upgrade-.quick_dra-only_version.yaml"sv,
	            },
	    },
	    {
	        .name = "bulk-upgrade employment history"sv,
	        .args = "config upgrade --config .quick_dra.yaml --on 2022/01"sv,
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
	        .stdout = "\033[0;90mConfig version updated\033[m\n"sv,
	        .writes =
	            new_file{
	                .name = ".quick_dra.yaml"sv,
	                .cmp = ".config-upgrade-.quick_dra-simple_history.yaml"sv,
	            },
	    },
	    {
	        .name = "do not upgrade 1 to 2"sv,
	        .args = "config upgrade --config .quick_dra.yaml --on 2022/01"sv,
	        .config =
	            R"($schema: 'https://raw.githubusercontent.com/mbits-os/quick_dra/refs/heads/main/data/schemas/user_config_schema.yaml'
wersja: 2
ubezpieczeni:
  - nazwisko: 'Iksiński, Piotr'
    dowód: ABC523456
    tytuł ubezpieczenia: 0110 0 0
    historia:
      2022/1:
        wymiar: 1/4
  - nazwisko: 'Iksiński, Jan'
    paszport: EH0123456
    tytuł ubezpieczenia: 0110 0 0
    historia:
      2022/1:
        wymiar: 3/4
        pensja: 9000 zł
  - nazwisko: 'Iksińska, Maria'
    tytuł ubezpieczenia: 0110 0 0
    pesel: 26211012346
    historia:
      2022/1:
        pensja: 7500 zł
wypadkowe: {}
)"sv,
	        .writes =
	            new_file{
	                .name = ".quick_dra.yaml"sv,
	                .cmp = ".config-upgrade-.quick_dra-simple_history.yaml"sv,
	            },
	    },

	    // accident [get]
	    {
	        .name = "get not contribution from empty config"sv,
	        .args = "config accident --config .quick_dra.yaml"sv,
	        .config = "wersja: 1"sv,
	    },
	    {
	        .name = "get accident contribution A"sv,
	        .args = "config accident --config .quick_dra.yaml --on 2026/01"sv,
	        .config = R"(wersja: 2
wypadkowe:
  2002/01: 1.67%
  2020/07: 20%
)"sv,
	        .stdout = "20% \033[0;90m[set in July 2020]\033[m\n"sv,
	    },
	    {
	        .name = "get accident contribution B"sv,
	        .args = "config accident --config .quick_dra.yaml --on 2010/01"sv,
	        .config = R"(wersja: 2
wypadkowe:
  2002/01: 1.67%
  2020/07: 20%
)"sv,
	        .stdout = "1.67% \033[0;90m[set in January 2002]\033[m\n"sv,
	    },
	    {
	        .name = "get accident contribution C"sv,
	        .args = "config accident --config .quick_dra.yaml --on 1999/01"sv,
	        .config = R"(wersja: 2
wypadkowe:
  2002/01: 1.67%
  2020/07: 20%
)"sv,
	    },
	    {
	        .name = "get accident contribution D"sv,
	        .args = "config accident --config .quick_dra.yaml --on 1999/01"sv,
	        .config = R"(wersja: 2
wypadkowe:
  0/1: 3.3%
  2002/01: 1.67%
  2020/07: 20%
)"sv,
	        .stdout = "3.3% \033[0;90m[set in \"default\" month]\033[m\n"sv,
	    },

	    // accident [set]
	    {
	        .name = "set contribution into (and upgrade) config v1"sv,
	        .args = "config accident --config .quick_dra.yaml --on 2026/4 1.67%"sv,
	        .config = "wersja: 1"sv,
	        .stdout = "\033[0;90mAccident contribution for April 2026 set to \033[m1.67%\n"
	                  "\033[0;90mConfig version updated\033[m\n"sv,
	        .writes =
	            new_file{
	                .name = ".quick_dra.yaml"sv,
	                .cmp = ".config-set-and-upgrade-.quick_dra-empty_config.yaml"sv,
	            },
	    },
	    {
	        .name = "set contribution into config v2"sv,
	        .args = "config accident --config .quick_dra.yaml --on 2026/4 1.67%"sv,
	        .config = "wersja: 2"sv,
	        .stdout = "\033[0;90mAccident contribution for April 2026 set to \033[m1.67%\n"sv,
	        .writes =
	            new_file{
	                .name = ".quick_dra.yaml"sv,
	                .cmp = ".config-set-and-upgrade-.quick_dra-empty_config.yaml"sv,
	            },
	    },

	    // unhappy paths:
	    {
	        .name = "upgrade has invalid date"sv,
	        .args = "config upgrade --config .quick_dra.yaml --on 2022/14"sv,
	        .config = "wersja: 1"sv,
	        .stderr = R"(usage: qdra config upgrade [-h] [--config <path>] [--on <yyyy/mm>]
qdra config upgrade: error: --on expected YYYY/MM, got `2022/14`
)"sv,
	        .returncode = 2,
	    },
	    {
	        .name = "accident get has invalid date"sv,
	        .args = "config accident --config .quick_dra.yaml --on 2022/14"sv,
	        .config = "wersja: 1"sv,
	        .stderr = R"(usage: qdra config accident [-h] [--config <path>] [--on <yyyy/mm>] [<percent>]
qdra config accident: error: --on expected YYYY/MM, got `2022/14`
)"sv,
	        .returncode = 2,
	    },
	    {
	        .name = "accident get has invalid year"sv,
	        .args = "config accident --config .quick_dra.yaml --on 1999/01"sv,
	        .config = R"(wersja: 2
wypadkowe:
  1996/15: 3.3%
)"sv,
	        .stderr =
	            R"(.quick_dra.yaml:3:3: error: expecting YYYY/MM or YYYY-MM
.quick_dra.yaml:1:1: error: while reading `wypadkowe`
Quick-DRA: error: .quick_dra.yaml needs to be updated before continuing
)"sv,
	        .returncode = 1,
	    },
	    {
	        .name = "accident set has no percent"sv,
	        .args = "config accident --config .quick_dra.yaml --on 2026/4 contribution"sv,
	        .config = "wersja: 2"sv,
	        .stderr = R"(usage: qdra config accident [-h] [--config <path>] [--on <yyyy/mm>] [<percent>]
qdra config accident: error: <percent>: expecting a number with 0.01 increment, with optional % suffix
)"sv,
	        .returncode = 2,
	    },
	};

	INSTANTIATE_TEST_SUITE_P(config, cli_test, ::testing::ValuesIn(tests));
}  // namespace quick_dra::builtin::testing
