// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/ranges.h>
#include <quick_dra/base/str.hpp>
#include "_parser_types.hpp"

namespace quick_dra::v1 {
	namespace {
		std::string from_rate(rate const& r) {
			if (r.insured)
				return fmt::format("{{ total: {}%, insured: {}% }}", r.total,
				                   *r.insured);
			return fmt::format("{{ total: {}% }}", r.total);
		}
	}  // namespace

	void config::debug_print() const noexcept {
		fmt::print("Insurer:\n");
		fmt::print("  Name: {} {}\n", insurer.first, insurer.last);
		fmt::print("  Social ID: {}\n", insurer.social_id);
		fmt::print("  Tax ID: {}\n", insurer.tax_id);
		fmt::print("  Ident (kind:{}): {}\n", insurer.kind, insurer.document);

		fmt::print("Insured ({}):\n", insured.size());
		for (auto const& obj : insured) {
			auto const scale = obj.part_time_scale.value_or(ratio{});

			fmt::print("  - Name: {} {}\n", obj.first, obj.last);
			fmt::print("    Insurance title: {}\n", obj.title.split());
			fmt::print("    Ident (kind:{}): {}\n", obj.kind, obj.document);
			fmt::print("    Scale: {}/{}\n", std::max(1u, scale.num),
			           std::max(1u, scale.den));
			fmt::print("    Remuneration: {}\n",
			           obj.remuneration
			               .transform([](auto const& value) {
				               return fmt::format("{} zł", value);
			               })
			               .value_or("<minimal>"));
		}

		fmt::print("Minimal ({}):\n", minimal.size());
		for (auto const& [date, amount] : minimal) {
			fmt::print("  {}-{:02}: {} zł\n", static_cast<int>(date.year()),
			           static_cast<unsigned>(date.month()), amount);
		}

		fmt::print("Parameters\n");
		fmt::print("  cost_of_obtaining: {} zł\n", params.cost_of_obtaining);
		fmt::print("  tax_free_allowance: {} zł\n", params.tax_free_allowance);
		fmt::print("  free_amount: {} zł\n", params.free_amount);
		fmt::print("  tax_rate: {}%\n", params.tax_rate);
		fmt::print("  health: {}%\n", params.health);
		fmt::print("  pension_insurance: {}\n",
		           from_rate(params.pension_insurance));
		fmt::print("  disability_insurance: {}\n",
		           from_rate(params.disability_insurance));
		fmt::print("  health_insurance: {}\n",
		           from_rate(params.health_insurance));
		fmt::print("  accident_insurance: {}\n",
		           from_rate(params.accident_insurance));
		fmt::print("  guaranteed_employee_benefits_fund: {}\n",
		           from_rate(params.guaranteed_employee_benefits_fund));
	}
}  // namespace quick_dra::v1

namespace quick_dra {
	void compiled_templates::debug_print() const noexcept {
		fmt::print("Reports ({}):\n", reports.size());

		for (auto const& [kedu, report] : reports) {
			fmt::print("  {}:\n", kedu);
			quick_dra::debug_print(report);
		}
	}
}  // namespace quick_dra
