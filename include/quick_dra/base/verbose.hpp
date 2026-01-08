// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

namespace quick_dra {
	enum class verbose : unsigned {
		none,
		names_only,
		names_and_summary,
		names_and_details,
		parameters,
		raw_form_data,
		templates,
		calculated_sections,
		last,
	};
}
