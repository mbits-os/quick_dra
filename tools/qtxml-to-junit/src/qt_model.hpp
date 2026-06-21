// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <chrono>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>
#include "common.hpp"

namespace fs = std::filesystem;

struct Message {
	bool isIncident{false};
	Type type{};
	std::string file{};
	unsigned line{};
	std::string tag{};
	std::string description{};

	static Message const* resultMessage(std::vector<Message> const& messages);
};

static inline std::string const& key_from(Message const& msg) noexcept { return msg.tag; }

template <typename T>
struct OrderedMultimap {
	using Map = std::map<std::string, std::vector<T>>;
	std::vector<std::string> order{};
	Map lookup{};

	void add(T&& value) {
		auto const& key = key_from(value);
		auto it = lookup.lower_bound(key);
		if (it == lookup.end() || it->first != key) {
			it = lookup.insert(it, {key, typename Map::mapped_type{}});
			order.push_back(key);
		}
		it->second.emplace_back(std::move(value));
	}
};

struct TestFunction {
	OrderedMultimap<Message> messages{};
	std::string name{};
	milliseconds_f duration{};
};

struct TestCase {
	std::string name{};
	std::vector<TestFunction> functions{};
	milliseconds_f duration{};

	static std::optional<TestCase> fromFile(fs::path const& path);
};
