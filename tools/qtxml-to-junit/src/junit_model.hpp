// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <vector>
#include "common.hpp"

struct stats {
	std::string name{};
	size_t tests{};
	size_t failures{};
	size_t skipped{};
	milliseconds_f time{};
	ch::sys_seconds timestamp{};

	void add(stats const& rhs) {
		tests += rhs.tests;
		failures += rhs.failures;
		skipped += rhs.skipped;
		time += rhs.time;
	}
};

struct testcase {
	std::string name{};
	std::string classname{};
	milliseconds_f time{};
	std::string file{};
	unsigned line{};
	Type type{};
	std::string message{};
	std::string output{};
};

struct testsuite {
	::stats stats{};
	std::vector<testcase> children{};

	void add(testcase&& test) {
		stats.time += test.time;
		++stats.tests;
		if (test.type == Type::failure) {
			++stats.failures;
		} else if (test.type == Type::skipped) {
			++stats.skipped;
		}
		children.emplace_back(std::move(test));
	}
};

struct testsuites {
	::stats stats{};
	std::vector<testsuite> children{};

	void add(testsuite&& suite) {
		stats.add(suite.stats);
		children.emplace_back(std::move(suite));
	}

	void propagate_timestamp(ch::sys_time<milliseconds_f> now) {
		using std::literals::operator""s;
		stats.timestamp = ch::floor<ch::seconds>(now + .5s);
		for (auto& child : children) {
			child.stats.timestamp = ch::floor<ch::seconds>(now + .5s);
			now += child.stats.time;
		}
	}
};