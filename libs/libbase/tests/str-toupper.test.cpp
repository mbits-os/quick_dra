// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <quick_dra/base/str.hpp>

namespace quick_dra::testing {
	namespace {
		using std::literals::operator""sv;
	}  // namespace

	struct one_case_testcase {
		std::string_view input{};
		std::string_view expected_to_upper{};
		std::string_view expected_to_lower{};

		friend std::ostream& operator<<(std::ostream& out, one_case_testcase const& test) {
			return out << '"' << test.input << "\" -> \"" << test.expected_to_upper << '"';
		}
	};

	class one_case : public ::testing::TestWithParam<one_case_testcase> {};

	TEST_P(one_case, to_upper) {
		auto const& [input, expected, _] = GetParam();
		auto const actual = to_upper(input);
		ASSERT_EQ(expected, actual);
	}

	TEST_P(one_case, to_lower) {
		auto const& [input, _, expected] = GetParam();
		auto const actual = to_lower(input);
		ASSERT_EQ(expected, actual);
	}

	static constinit one_case_testcase const tests[] = {
	    {"coöperate"sv, "COÖPERATE"sv, "coöperate"sv},
	    {"Iksiński"sv, "IKSIŃSKI"sv, "iksiński"sv},
	    {"Zażółć gęślą jaźń"sv, "ZAŻÓŁĆ GĘŚLĄ JAŹŃ"sv, "zażółć gęślą jaźń"sv},
	    {"Pchnąć w tę łódź jeża lub ośm skrzyń fig."sv, "PCHNĄĆ W TĘ ŁÓDŹ JEŻA LUB OŚM SKRZYŃ FIG."sv,
	     "pchnąć w tę łódź jeża lub ośm skrzyń fig."sv},
	    {
	        "Γαζίες καὶ μυρτιὲς δὲν θὰ βρῶ πιὰ στὸ χρυσαφὶ ξέφωτο."sv,
#ifdef WIN32
	        "ΓΑΖΊΕς ΚΑῚ ΜΥΡΤΙῈς ΔῈΝ ΘᾺ ΒΡῶ ΠΙᾺ ΣΤῸ ΧΡΥΣΑΦῚ ΞΈΦΩΤΟ."sv,
#else
	        "ΓΑΖΊΕΣ ΚΑῚ ΜΥΡΤΙῈΣ ΔῈΝ ΘᾺ ΒΡΩ͂ ΠΙᾺ ΣΤῸ ΧΡΥΣΑΦῚ ΞΈΦΩΤΟ."sv,
#endif
	        "γαζίες καὶ μυρτιὲς δὲν θὰ βρῶ πιὰ στὸ χρυσαφὶ ξέφωτο."sv,
	    },
	    {
	        "Zwölf große Boxkämpfer jagen Viktor quer über den Sylter Deich."sv,
#ifdef WIN32
	        "ZWÖLF GROßE BOXKÄMPFER JAGEN VIKTOR QUER ÜBER DEN SYLTER DEICH."sv,
#else
	        "ZWÖLF GROSSE BOXKÄMPFER JAGEN VIKTOR QUER ÜBER DEN SYLTER DEICH."sv,
#endif
	        "zwölf große boxkämpfer jagen viktor quer über den sylter deich."sv,
	    },
	    {"Любя, съешь щипцы, — вздохнёт мэр, — кайф жгуч."sv, "ЛЮБЯ, СЪЕШЬ ЩИПЦЫ, — ВЗДОХНЁТ МЭР, — КАЙФ ЖГУЧ."sv,
	     "любя, съешь щипцы, — вздохнёт мэр, — кайф жгуч."sv},
	};

	INSTANTIATE_TEST_SUITE_P(test, one_case, ::testing::ValuesIn(tests));

}  // namespace quick_dra::testing
