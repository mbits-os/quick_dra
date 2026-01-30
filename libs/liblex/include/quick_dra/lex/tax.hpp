// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <map>
#include <quick_dra/base/types.hpp>

namespace quick_dra {
	currency calc_tax_lowering_amount(
	    std::map<currency, percent> const& scale) noexcept;
	currency calc_tax_owed(std::map<currency, percent> const& scale,
	                       currency taxable_amount) noexcept;
};  // namespace quick_dra
