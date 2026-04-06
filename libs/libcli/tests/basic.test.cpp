// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <quick_dra/version.hpp>
#include "run.hpp"

namespace quick_dra::builtin::testing {
	static constexpr runnable_testcase tests[] = {
	    {
	        .name = "empty argument list"sv,
	        .args = ""sv,
	        .stdout = R"(usage: qdra [-h] [--version] <command> [<args>]

optional arguments:
 -h, --help    shows this help message and exits
 -v, --version show version and exit

known commands:
 xml           produce KEDU 5.6 XML file
 list          list people in configuration
 payer         manage the payer data in ~/.quick_dra.yaml file
 insured       manage the insured data in ~/.quick_dra.yaml file
)"sv,
	    },
	    {
	        .name = "unknown param"sv,
	        .args = "--force"sv,
	        .stderr =
	            R"(usage: qdra [-h] [--version] <command> [<args>]
qdra: error: unrecognized argument: --force
)"sv,
	        .returncode = 2,
	    },
	    {
	        .name = "help"sv,
	        .args = "--help"sv,
	        .stdout = R"(usage: qdra [-h] [--version] <command> [<args>]

optional arguments:
 -h, --help    shows this help message and exits
 -v, --version show version and exit

known commands:
 xml           produce KEDU 5.6 XML file
 list          list people in configuration
 payer         manage the payer data in ~/.quick_dra.yaml file
 insured       manage the insured data in ~/.quick_dra.yaml file
)"sv,
	    },
	    {
	        .name = "version"sv,
	        .args = "--version"sv,
	        .stdout = "Quick-DRA version " QUICK_DRA_VERSION_STR "\n"sv,
	    },
	    {
	        .name = "unknown subcommand"sv,
	        .args = "noent"sv,
	        .stderr =
	            R"(usage: qdra [-h] [--version] <command> [<args>]
qdra: "noent" is not a qdra command

known commands:
 xml     produce KEDU 5.6 XML file
 list    list people in configuration
 payer   manage the payer data in ~/.quick_dra.yaml file
 insured manage the insured data in ~/.quick_dra.yaml file
)"sv,
	        .returncode = 1,
	    },
	};

	INSTANTIATE_TEST_SUITE_P(basic, cli_test, ::testing::ValuesIn(tests));
}  // namespace quick_dra::builtin::testing
