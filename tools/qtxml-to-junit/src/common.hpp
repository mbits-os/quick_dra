// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <chrono>

namespace ch = std::chrono;

enum class Type {
	pass,
	skipped,
	failure,
	other,
};

using milliseconds_f = ch::duration<double, std::milli>;
// using seconds_f = ch::duration<double, std::ratio<1>>;
