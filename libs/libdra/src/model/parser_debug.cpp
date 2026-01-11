// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/ranges.h>
#include <quick_dra/base/str.hpp>
#include "_parser_types.hpp"

namespace quick_dra::v1 {
	namespace {
		std::string from_rate(rate const& r) {
			if (r.insured)
				return fmt::format("total {}%, insured {}%", r.total,
				                   *r.insured);
			return fmt::format("{}%", r.total);
		}
	}  // namespace

	void config::debug_print(verbose level) const noexcept {
		if (level == verbose::none) {
			return;
		}

		if (level == verbose::names_only) {
			fmt::print("-- insurer: {} {}\n", insurer.first, insurer.last);
			fmt::print("-- insured:\n");
			for (auto const& obj : insured) {
				fmt::print("--   - {} {}\n", obj.first, obj.last);
			}
		} else if (level == verbose::names_and_summary) {
			fmt::print("-- insurer: {} {} ({})\n", insurer.first, insurer.last,
			           insurer.tax_id);
			fmt::print("-- insured:\n");
			for (auto const& obj : insured) {
				auto const scale = obj.part_time_scale.value_or(ratio{});
				fmt::print("--   - {} {} ({}), {}/{} of {}\n", obj.first,
				           obj.last, obj.document, std::max(1u, scale.num),
				           std::max(1u, scale.den),
				           obj.remuneration
				               .transform([](auto const& value) {
					               return fmt::format("{} zł", value);
				               })
				               .value_or("<minimal pay>"));
			}
			return;
		} else {
			fmt::print("-- insurer:\n");
			fmt::print("--   name: {} {}\n", insurer.first, insurer.last);
			fmt::print("--   social id: {}\n", insurer.social_id);
			fmt::print("--   tax id: {}\n", insurer.tax_id);
			fmt::print("--   ident: {} {}\n", insurer.kind, insurer.document);

			fmt::print("-- insured:\n");
			for (auto const& obj : insured) {
				auto const scale = obj.part_time_scale.value_or(ratio{});

				fmt::print("--   - name: {} {}\n", obj.first, obj.last);
				fmt::print("--     insurance title: {}\n",
				           fmt::join(obj.title.split(), " "));
				fmt::print("--     ident: {} {}\n", obj.kind, obj.document);
				fmt::print("--     remuneration: {}/{} of {}\n",
				           std::max(1u, scale.num), std::max(1u, scale.den),
				           obj.remuneration
				               .transform([](auto const& value) {
					               return fmt::format("{} zł", value);
				               })
				               .value_or("<minimal pay>"));
			}
		}

		if (level < verbose::parameters) {
			return;
		}

		fmt::print("-- minimal pay per month:\n");
		for (auto const& [date, amount] : minimal) {
			fmt::print("--   {}-{:02}: {} zł\n", static_cast<int>(date.year()),
			           static_cast<unsigned>(date.month()), amount);
		}

		fmt::print("-- parameters\n");
		fmt::print("--   cost of obtaining: {} zł\n", params.cost_of_obtaining);
		fmt::print("--   tax-free allowance: {} zł\n",
		           params.tax_free_allowance);
		fmt::print("--   free amount: {} zł\n", params.free_amount);
		fmt::print("--   tax rate: {}%\n", params.tax_rate);
		fmt::print("--   health: {}%\n", params.health);
		fmt::print("--   pension insurance: {}\n",
		           from_rate(params.pension_insurance));
		fmt::print("--   disability insurance: {}\n",
		           from_rate(params.disability_insurance));
		fmt::print("--   health insurance: {}\n",
		           from_rate(params.health_insurance));
		fmt::print("--   accident insurance: {}\n",
		           from_rate(params.accident_insurance));
		fmt::print("--   guaranteed employee benefits fund: {}\n",
		           from_rate(params.guaranteed_employee_benefits_fund));
	}
}  // namespace quick_dra::v1

namespace quick_dra {
	void compiled_templates::debug_print() const noexcept {
		fmt::print("-- templates:\n");

		for (auto const& [kedu, report] : reports) {
			fmt::print("--   [{}]:\n", kedu);
			quick_dra::debug_print(report);
		}
	}
}  // namespace quick_dra
