// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <quick_dra/docs/xml.hpp>
#include <quick_dra/docs/xml_builder.hpp>
#include <sstream>

namespace quick_dra::testing {
	namespace {
		enum class indent { none, tab, two_spaces };
		std::string format(xml const& root, indent indented = indent::none) {
			std::ostringstream out;
			switch (indented) {
				case indent::tab:
					out << root.indented();
					break;
				case indent::two_spaces:
					out << root.indented("  "sv);
					break;
				default:
				case indent::none:
					out << root;
					break;
			}
			return std::move(out).str();
		}
		xml test_tree() {
			auto child =
			    E("child"sv, {{"quoted"s, "before ' between \" after"}})
			        .with("text"sv)
			        .with("<code>&ref</code>"sv);
			return E("root"sv, {{"version"s, "1"}}).with(child);
		}
	}  // namespace

	TEST(xml, indented) {
		auto const actual = format(test_tree(), indent::tab);
		auto const expected =
		    "<root version=\"1\">\n"
		    ""
		    "\t<child quoted=\"before &#39; between &quot; "
		    "after\">&lt;code&gt;&amp;ref&lt;/code&gt;</child>\n"
		    ""
		    "</root>\n"
		    ""
		    ""sv;
		ASSERT_EQ(actual, expected);
	}

	TEST(xml, terse) {
		auto const actual = format(test_tree());
		auto const expected =
		    "<root version=\"1\">"
		    "<child quoted=\"before &#39; between &quot; "
		    "after\">&lt;code&gt;&amp;ref&lt;/code&gt;</child>"
		    "</root>"sv;
		ASSERT_EQ(actual, expected);
	}

	TEST(xml, swap) {
		auto root = E("root");
		root.with("3 < 4"sv);

		auto actual = format(root);
		auto expected = "<root>3 &lt; 4</root>"sv;
		ASSERT_EQ(actual, expected);
		ASSERT_TRUE(root.has_text());
		ASSERT_EQ(root.text(), "3 < 4"sv);

		root.with(E("a"sv, {{"href"s, "https://example.com/"s}}));
		actual = format(root);
		expected =
		    "<root><a href=\"https://example.com/\"><!-- empty --></a></root>"sv;
		ASSERT_EQ(actual, expected);
		ASSERT_FALSE(root.has_text());
		ASSERT_EQ(root.children().size(), 1);
		ASSERT_EQ(root.children().front().tag, "a"sv);

		actual = format(root, indent::tab);
		expected =
		    "<root>\n"
		    ""
		    "\t<a href=\"https://example.com/\"><!-- empty --></a>\n"
		    ""
		    "</root>\n"
		    ""sv;
		ASSERT_EQ(actual, expected);

		root.with("inner text"sv);
		actual = format(root);
		expected = "<root>inner text</root>"sv;
		ASSERT_EQ(actual, expected);

		auto ref = E("strong").with("<!-- bolded text -->"sv);
		root.with(ref);
		actual = format(root);
		expected = "<root><strong>&lt;!-- bolded text --&gt;</strong></root>"sv;
		ASSERT_EQ(actual, expected);
	}

	TEST(xml, builder) {
		auto root = build_kedu_doc("app"sv, "1.0"sv);
		auto form = quick_dra::form{
		    .key{"TEST"s},
		    .state{},
		};

		form.state.insert("name.property"_var, 0.99_PLN);

		auto tmplt = std::vector{
		    compiled_section{
		        .id = "I"s,
		        .blocks =
		            {
		                compiled_block{
		                    .id{},
		                    .fields{
		                        {1, {}},
		                        {2, "label"s},
		                        {3, 100_PLN},
		                        {4, 99_per},
		                        {6, 10_PLN},
		                        {7, 2000_PLN},
		                        {8, 1999_PLN},
		                        {9, "name.property"_var},
		                        {10, "name.non_existent"_var},
		                        {21, addition{.refs = {3, 6, 7}}},
		                        {22, addition{.refs = {3, 4, 6}}},
		                        {23,
		                         std::vector<compiled_value>{
		                             uint_value{3},
		                             15_PLN,
		                             "another label"s,
		                         }},
		                    },
		                },
		            },
		    },
		    compiled_section{
		        .id = "II"s,
		        .blocks =
		            {
		                compiled_block{.id{"A"s}, .fields{{1, "\"quoted\""s}}},
		                compiled_block{.id{"B"s},
		                               .fields{{1, 1_per}, {2, 1.99_PLN}}},
		                compiled_block{.id{},
		                               .fields{{1, 1_per}, {2, 1.99_PLN}}},
		            },
		    },
		};

		::testing::internal::CaptureStderr();
		attach_document(root, verbose::none, form, tmplt, 1001);
		auto const log = ::testing::internal::GetCapturedStderr();

		auto const actual = format(root, indent::two_spaces);
		auto const expected =
		    "<KEDU wersja_schematu=\"1\" "
		    "xmlns=\"http://www.zus.pl/2024/KEDU_5_6\">\n"
		    "  <naglowek.KEDU>\n"
		    "    <program>\n"
		    "      <producent>midnightBITS</producent>\n"
		    "      <symbol>app</symbol>\n"
		    "      <wersja>1.0</wersja>\n"
		    "    </program>\n"
		    "  </naglowek.KEDU>\n"
		    "  <ZUSTEST id_dokumentu=\"1001\">\n"
		    "    <I>\n"
		    "      <p2>label</p2>\n"
		    "      <p3>100.00</p3>\n"
		    "      <p4>99.00</p4>\n"
		    "      <p6>10.00</p6>\n"
		    "      <p7>2000.00</p7>\n"
		    "      <p8>1999.00</p8>\n"
		    "      <p9>0.99</p9>\n"
		    "      <p21>2110.00</p21>\n"
		    "      <p23>\n"
		    "        <p1>3</p1>\n"
		    "        <p2>15.00</p2>\n"
		    "        <p3>another label</p3>\n"
		    "      </p23>\n"
		    "    </I>\n"
		    "    <II>\n"
		    "      <A>\n"
		    "        <p1>&quot;quoted&quot;</p1>\n"
		    "      </A>\n"
		    "      <B>\n"
		    "        <p1>1.00</p1>\n"
		    "        <p2>1.99</p2>\n"
		    "      </B>\n"
		    "      <p1>1.00</p1>\n"
		    "      <p2>1.99</p2>\n"
		    "    </II>\n"
		    "  </ZUSTEST>\n"
		    "</KEDU>\n"sv;
		auto const expected_log =
		    "I p10: error: cannot find `$name.non_existent'\n"
		    "I p22: error: p4 is not a number\n"sv;
		ASSERT_EQ(actual, expected);
		ASSERT_EQ(log, expected_log);
	}
}  // namespace quick_dra::testing
