// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <chrono>
#include <quick_dra/docs/xml.hpp>
#include <quick_dra/io/options.hpp>
#include <quick_dra/model/types.hpp>
#include <string>

namespace quick_dra {
	xml build_file_set(options const& opts,
	                   config const& cfg,
	                   compiled_templates const& templates);
}
