// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <quick_dra/docs/xml.hpp>
#include <sstream>

namespace quick_dra::testing {
	namespace {
		std::string format(xml const& root, bool indented = false) {
			std::ostringstream out;
			if (indented)
				out << root.indented();
			else
				out << root;
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
		auto const actual = format(test_tree(), true);
		auto const expected =
		    "<root version=\"1\">\n"
		    "\t<child quoted=\"before &#39; between &quot; "
		    "after\">&lt;code&gt;&amp;ref&lt;/code&gt;</child>\n"
		    "</root>\n"
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

}  // namespace quick_dra::testing
