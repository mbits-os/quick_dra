// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <quick_dra/base/str.hpp>

namespace quick_dra::testing {
	namespace {
		using std::literals::operator""sv;
	}  // namespace

	struct uppercase_testcase {
		std::string_view input{};
		std::string_view expected{};

		friend std::ostream& operator<<(std::ostream& out,
		                                uppercase_testcase const& test) {
			return out << '"' << test.input << "\" -> \"" << test.expected
			           << '"';
		}
	};

	class uppercase : public ::testing::TestWithParam<uppercase_testcase> {};

	TEST_P(uppercase, conv) {
		auto const& [input, expected] = GetParam();
		auto const actual = to_upper(input);
		ASSERT_EQ(expected, actual);
	}

	static constinit uppercase_testcase const tests[] = {
	    {"coöperate"sv, "COÖPERATE"sv},
	    {"Iksiński"sv, "IKSIŃSKI"sv},
	    {"Zażółć gęślą jaźń"sv, "ZAŻÓŁĆ GĘŚLĄ JAŹŃ"sv},
	    {"Pchnąć w tę łódź jeża lub ośm skrzyń fig."sv,
	     "PCHNĄĆ W TĘ ŁÓDŹ JEŻA LUB OŚM SKRZYŃ FIG."sv},
	    {
	        "Γαζίες καὶ μυρτιὲς δὲν θὰ βρῶ πιὰ στὸ χρυσαφὶ ξέφωτο."sv,
#ifdef WIN32
	        "ΓΑΖΊΕς ΚΑῚ ΜΥΡΤΙῈς ΔῈΝ ΘᾺ ΒΡῶ ΠΙᾺ ΣΤῸ ΧΡΥΣΑΦῚ ΞΈΦΩΤΟ."sv,
#else
	        "ΓΑΖΊΕΣ ΚΑῚ ΜΥΡΤΙῈΣ ΔῈΝ ΘᾺ ΒΡΩ͂ ΠΙᾺ ΣΤῸ ΧΡΥΣΑΦῚ ΞΈΦΩΤΟ."sv,
#endif
	    },
	    {
	        "Zwölf große Boxkämpfer jagen Viktor quer über den Sylter Deich."sv,
#ifdef WIN32
	        "ZWÖLF GROßE BOXKÄMPFER JAGEN VIKTOR QUER ÜBER DEN SYLTER DEICH."sv,
#else
	        "ZWÖLF GROSSE BOXKÄMPFER JAGEN VIKTOR QUER ÜBER DEN SYLTER DEICH."sv,
#endif
	    },
	    {"Любя, съешь щипцы, — вздохнёт мэр, — кайф жгуч."sv,
	     "ЛЮБЯ, СЪЕШЬ ЩИПЦЫ, — ВЗДОХНЁТ МЭР, — КАЙФ ЖГУЧ."sv},
	};

	INSTANTIATE_TEST_SUITE_P(test, uppercase, ::testing::ValuesIn(tests));

}  // namespace quick_dra::testing
