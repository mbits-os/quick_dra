// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <quick_dra/models/types.hpp>
#include <yaml/parser.hpp>
#include <yaml/reader.hpp>
#include <yaml/ref.hpp>

namespace quick_dra {
	using namespace yaml;
	bool read_value(ref_ctx const& ref, percent& ctx);
	bool read_value(ref_ctx const& ref, currency& ctx);
	bool read_value(ref_ctx const& ref, ratio& ctx);
	bool read_value(ref_ctx const& ref, insurance_title& ctx);
}  // namespace quick_dra

namespace yaml {
	bool convert_string(ref_ctx const& ref,
	                    c4::csubstr const& value,
	                    std::chrono::year_month& ctx);
}
