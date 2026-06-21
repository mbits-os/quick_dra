// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <tinyxml2.h>
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <map>
#include <memory>
#include <quick_dra/base/str.hpp>
#include <quick_dra/docs/xml.hpp>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include "junit_model.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4702)
#endif

#include <args/parser.hpp>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

using namespace std::literals;
using namespace quick_dra;
using namespace tinyxml2;

namespace fs = std::filesystem;
namespace ch = std::chrono;

namespace {
	std::string str(auto value) { return std::format("{}", value); }

	struct attr_builder {
		std::map<std::string, std::string> attrs{};
		attr_builder& add(std::string const& name, std::string_view value) {
			if (value.empty()) return *this;
			attrs[name] = as_str(value);
			return *this;
		}

		attr_builder& add(std::string const& name, size_t value) {
			attrs[name] = str(value);
			return *this;
		}

		attr_builder& add(std::string const& name, milliseconds_f time) {
			attrs[name] = str(time.count());
			return *this;
		}

		attr_builder& add(std::string const& name, ch::system_clock::time_point value) {
			if (value == ch::system_clock::time_point{}) return *this;
			attrs[name] = std::format("{0:%F}T{0:%H:%M:%S%z}", ch::floor<ch::seconds>(value));
			return *this;
		}

		std::map<std::string, std::string> value() && { return std::move(attrs); }
	};

	std::map<std::string, std::string> to_attr(::stats const& stats) {
#define attr(NAME) add(#NAME ""s, stats.NAME)
		auto attrs = attr_builder{}.attr(name).attr(tests).attr(failures).attr(skipped).attr(time).attr(timestamp);
		return std::move(attrs).value();
#undef attr
	}

	quick_dra::xml to_xml(testsuites const& suites) {
		auto root = E("testsuites"sv, to_attr(suites.stats));
		for (auto const& suite : suites.children) {
			auto suiteElem = E("testsuite"sv, to_attr(suite.stats));
			for (auto const& test : suite.children) {
#define attr(NAME) add(#NAME ""s, test.NAME)
				auto attrs = attr_builder{}.attr(name).attr(classname).attr(file).attr(time);
				if (!test.file.empty()) attrs.attr(line);
#undef attr
				auto testElem = E("testcase"sv, attrs.attrs);
				if (test.type == Type::failure) {
					testElem.with(E("failure"sv, {{"message"s, test.message}}));
				} else if (test.type == Type::skipped) {
					testElem.with(E("skipped"sv, {{"message"s, test.message}}));
				}
				if (!test.output.empty()) {
					testElem.with(E("system-out"sv).with(test.output));
				}
				suiteElem.with(testElem);
			}
			root.with(suiteElem);
		}
		return root;
	}

	struct closed_elements_t {
		xml const& ref{};
		std::string_view indentation{};
		size_t level{};

		closed_elements_t child(xml const& child) const { return {child, indentation, level + 1}; }

		void indent(std::ostream& os) const {
			for (size_t index = 0; index < level; ++index)
				os << indentation;
		}

		friend std::ostream& operator<<(std::ostream& os, closed_elements_t const& node) {
			node.indent(os);
			fmt::print(os, "<{}", node.ref.tag);
			node.ref.print_attributes(os);

			if (std::holds_alternative<std::string>(node.ref.inside)) {
				fmt::print(os, ">{}", xml_escape(std::get<std::string>(node.ref.inside)));
				node.ref.print_close_tag(os);
			} else {
				auto const& list = std::get<xml::vector>(node.ref.inside);
				if (list.empty()) {
					os << " />";
				} else {
					os << ">\n";
					for (auto const& item : list) {
						os << node.child(item);
					}
					node.indent(os);
					node.ref.print_close_tag(os);
				}
			}

			return os << '\n';
		}
	};

	closed_elements_t close_empty_elements(xml const& root, std::string_view indentation) {
		return {root, indentation};
	}
}  // namespace

std::optional<testsuite> load_qt_xml(fs::path const&);

int convert(::args::args_view const& args) {
	std::vector<fs::path> inputs{};
	fs::path output{};
	args::null_translator tr{};
	args::parser parser{""s, args, &tr};
	parser.arg(inputs).meta("INPUT").help("filename of a file generated with `-o INPUT,xml` call to QTest testsuite");
	parser.arg(output, "o").meta("OUTPUT").help("filename of a resulting file");
	parser.parse();

	testsuites suites{};
	suites.children.reserve(inputs.size());
	for (auto const& input : inputs) {
		auto suite = load_qt_xml(input);
		if (!suite) {
			return 1;
		}
		suites.add(std::move(suite).value());
	}
	suites.propagate_timestamp(ch::system_clock::now());

	fs::create_directories(fs::weakly_canonical(fs::current_path() / output).parent_path());
	std::ofstream oss{output};
	oss << close_empty_elements(to_xml(suites), "  "sv);
	return 0;
}
