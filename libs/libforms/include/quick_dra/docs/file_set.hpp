// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <chrono>
#include <quick_dra/docs/forms.hpp>
#include <quick_dra/docs/xml.hpp>
#include <quick_dra/io/options.hpp>
#include <quick_dra/models/types.hpp>
#include <string>
#include <vector>

namespace quick_dra {
	xml build_file_set(options const& opts,
	                   std::vector<form> const& forms,
	                   compiled_templates const& templates);

	inline xml build_file_set(options const& opts,
	                          config const& cfg,
	                          compiled_templates const& templates) {
		auto const forms = prepare_form_set(
		    opts.verbose_level, opts.report_index, opts.date, opts.today, cfg);
		return build_file_set(opts, forms, templates);
	}
}  // namespace quick_dra
