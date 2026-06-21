// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <tinyxml2.h>
#include <filesystem>
#include <fstream>
#include <quick_dra/base/str.hpp>
#include <sstream>
#include "junit_model.hpp"
#include "qt_model.hpp"

using namespace quick_dra;
using namespace std::literals;
using namespace tinyxml2;

namespace fs = std::filesystem;
namespace ch = std::chrono;

namespace {
	std::string attribute(XMLElement* e, char const* name) {
		auto val = e->Attribute(name);
		return val ? val : "";
	}

	Type attr2Type(std::string_view type) {
		if (type == "skip"sv) return Type::skipped;
		if (type == "fail"sv || type == "xpass"sv || type == "bfail"sv || type == "bxpass"sv) return Type::failure;
		if (type == "pass"sv || type == "xfail"sv || type == "bpass"sv || type == "bxfail"sv) return Type::pass;
		return Type::other;
	}

	void children(XMLNode* n, auto&& cb) {
		auto child = n->FirstChild();
		while (child) {
			cb(child);
			child = child->NextSibling();
		}
	}

	void elements(XMLNode* n, auto&& cb) {
		auto child = n->FirstChildElement();
		while (child) {
			cb(child);
			child = child->NextSiblingElement();
		}
	}

	std::string text(XMLNode* n) {
		if (n->ToText()) return n->Value();
		std::string result{};
		children(n, [&result](XMLNode* child) { result += text(child); });
		return result;
	}

	std::optional<std::string> file_contents(fs::path const& path) {
		auto file = std::ifstream{path, std::ios::in | std::ios::binary};
		if (!file.good()) return std::nullopt;
		auto str = std::ostringstream{};
		str << file.rdbuf();
		return std::move(str).str();
	}

	Message MessageFromXml(XMLElement* e, bool isIncident) {
		Message result{.isIncident = isIncident,
		               .type = attr2Type(attribute(e, "type")),
		               .file = attribute(e, "file"),
		               .line = e->UnsignedAttribute("line")};

		elements(e, [&result](XMLElement* child) {
			if (child->Value() == "Description"sv) {
				result.description = text(child);
				static constexpr auto offscreen_warning = "This plugin does not support propagateSizeHints()"sv;
				if (result.description == offscreen_warning) result.description.clear();
			} else if (child->Value() == "DataTag"sv) {
				result.tag = text(child);
			}
		});

		return result;
	}

	TestFunction TestFunctionFromXml(XMLElement* e) {
		TestFunction result{.name = attribute(e, "name")};

		elements(e, [&result](XMLElement* child) {
			auto const tag = std::string_view{child->Value()};
			if (tag == "Incident"sv)
				result.messages.add(MessageFromXml(child, true));
			else if (tag == "Message"sv)
				result.messages.add(MessageFromXml(child, false));
			else if (tag == "Duration") {
				result.duration = milliseconds_f{child->DoubleAttribute("msecs")};
			}
		});

		return result;
	}

	TestCase TestCaseFromXml(XMLElement* e) {
		TestCase result{.name = attribute(e, "name")};

		elements(e, [&result](XMLElement* child) {
			auto const tag = std::string_view{child->Value()};
			if (tag == "TestFunction"sv) {
				result.functions.emplace_back(TestFunctionFromXml(child));
			} else if (tag == "Duration"sv) {
				result.duration = milliseconds_f{child->DoubleAttribute("msecs")};
			}
		});
		return result;
	}

	testsuite suite(TestCase const& tests) {
		testsuite result{.stats = {.name = tests.name}};

		size_t testcount = 0;
		for (auto const& function : tests.functions) {
			testcount += function.messages.order.size();
		}
		result.children.reserve(testcount);

		for (auto const& function : tests.functions) {
			auto const duration = function.duration / function.messages.order.size();

			for (auto const& key : function.messages.order) {
				auto it = function.messages.lookup.find(key);
				if (it == function.messages.lookup.end()) continue;
				auto const& [tag, messages] = *it;

				auto const definition = Message::resultMessage(messages);

				testcase test{
				    .name = function.name,
				    .classname = tests.name,
				    .time = duration,
				    .file = definition ? definition->file : ""s,
				    .line = definition ? definition->line : 0u,
				    .type = definition ? definition->type : Type::pass,
				    .message = definition ? definition->description : ""s,
				};

				if (!tag.empty()) test.name = std::format("{}({})", test.name, tag);

				for (auto const& message : messages) {
					if (&message == definition || message.description.empty()) continue;
					if (!test.output.empty()) test.output.push_back('\n');
					test.output.append(message.description);
				}

				result.add(std::move(test));
			}
		}

		if (result.stats.time < tests.duration) result.stats.time = tests.duration;

		return result;
	}
}  // namespace

// The first of skip, Fail, XPass or the blacklisted equivalents of the last two to arise is decisive for the
// outcome of the test.
Message const* Message::resultMessage(std::vector<Message> const& messages) {
	Message const* lastIncident = nullptr;
	for (auto const& msg : messages) {
		if (msg.isIncident) {
			lastIncident = &msg;
		}
		switch (msg.type) {
			case Type::skipped:
			case Type::failure:
				return &msg;
			default:
				break;
		}
	}
	return lastIncident;
}

std::optional<TestCase> TestCase::fromFile(fs::path const& input) {
	std::optional<TestCase> result{};

	auto input_xml_text = file_contents(input);
	if (!input_xml_text) {
		fmt::print(stderr, "error: cannot open {}\n", as_sv(input.u8string()));
		return result;
	}

	auto const document = std::make_unique<XMLDocument>();
	auto status = document->Parse(input_xml_text->c_str(), input_xml_text->size());
	if (status) {
		auto error = std::string_view{document->ErrorStr()};
		auto pos = error.find(" Line number="sv);
		if (pos != std::string_view::npos) {
			pos = error.find(": "sv, pos);
		}
		if (pos == std::string_view::npos) {
			error = ""sv;
		} else {
			error = error.substr(pos + 2);
		}
		fmt::print(stderr, "{}:{}: error {}: {}\n", as_sv(input.u8string()), document->ErrorLineNum(),
		           document->ErrorIDToName(status), error);
		return result;
	}

	result.emplace(TestCaseFromXml(document->RootElement()));
	return result;
}

std::optional<testsuite> load_qt_xml(fs::path const& input) { return TestCase::fromFile(input).transform(suite); }
