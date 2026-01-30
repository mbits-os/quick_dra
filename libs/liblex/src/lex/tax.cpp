// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <quick_dra/lex/tax.hpp>

namespace quick_dra {
	currency calc_tax_lowering_amount(
	    std::map<currency, percent> const& scale) noexcept {
		if (!scale.empty()) {
			auto const [amount, tax_rate] = *scale.begin();
			return calc_currency{(amount.calc() * tax_rate).value / 12}
			    .rounded();
		}

		return {};
	}

	currency calc_tax_owed(std::map<currency, percent> const& scale,
	                       currency taxable_amount) noexcept {
		auto const calc_taxable_amount_yearly =
		    calc_currency{taxable_amount.calc().value * 12};

		calc_currency tax_owed{};
		auto prev_rate = percent{};
		auto prev_amount = calc_currency{};

		auto const update = [&](calc_currency amount) {
			auto const difference = amount - prev_amount;
			tax_owed = tax_owed + difference * prev_rate;
		};

		for (auto const& [amount, tax_rate] : scale) {
			auto const calc_amount = amount.calc();
			if (calc_amount > calc_taxable_amount_yearly) {
				break;
			}

			update(calc_amount);

			prev_rate = tax_rate;
			prev_amount = calc_amount;
		}

		update(calc_taxable_amount_yearly);

		return calc_currency{(tax_owed).value / 12}.rounded();
	}
};  // namespace quick_dra
