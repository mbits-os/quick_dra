// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <quick_dra/models/types.hpp>
#include <yaml/parser.hpp>
#include <yaml/reader.hpp>
#include <yaml/ref.hpp>
#include <yaml/writer.hpp>

namespace quick_dra {
	using namespace yaml;
	bool read_value(ref_ctx const& ref, percent& ctx);
	bool read_value(ref_ctx const& ref, currency& ctx);
	bool read_value(ref_ctx const& ref, ratio& ctx);
	bool read_value(ref_ctx const& ref, insurance_title& ctx);
	bool read_value(ref_ctx const& ref, costs_of_obtaining& ctx);
	bool read_value(ref_ctx const& ref, rates& ctx);
	void write_value(ryml::NodeRef& ref, currency const& ctx);
	void write_value(ryml::NodeRef& ref, ratio const& ctx);
	void write_value(ryml::NodeRef& ref, insurance_title const& ctx);

	bool convert_string(ref_ctx const& ref, c4::csubstr const& value, currency& ctx);

	void upgrade(v1::config&& src, v2::config& dst);
	void upgrade(v1::partial::config&& src, v2::partial::config& dst);
	void downgrade(v2::partial::config const& src, v1::partial::config& dst);
}  // namespace quick_dra

namespace yaml {
	bool convert_string(ref_ctx const& ref, c4::csubstr const& value, std::chrono::year_month& ctx);
	bool convert_string(std::string_view value, std::chrono::year_month& ctx);
}  // namespace yaml
