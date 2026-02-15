// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/ranges.h>
#include <quick_dra/base/str.hpp>
#include <quick_dra/models/types.hpp>

namespace quick_dra::v1 {
	namespace {
		std::string from_rate(rate const& r) {
			std::vector<std::string> result{};
			result.reserve(2);
			if (r.payer != percent{}) {
				result.push_back(fmt::format("payer {}%", r.payer));
			}
			if (r.insured != percent{}) {
				result.push_back(fmt::format("insured {}%", r.insured));
			}
			// GCOV_EXCL_START
			if (result.empty()) {
				[[unlikely]];
				result.push_back("payer 0%"s);
			}
			// GCOV_EXCL_STOP
			return join(result, ", "_sep);
		}
	}  // namespace

	void config::debug_print(verbose level) const noexcept {
		if (level == verbose::none) {
			return;
		}

		if (level == verbose::names_only) {
			fmt::print("-- payer: {} {}\n", payer.first_name, payer.last_name);
			fmt::print("-- insured:\n");
			for (auto const& obj : insured) {
				fmt::print("--   - {} {}\n", obj.first_name, obj.last_name);
			}
		} else if (level == verbose::names_and_summary) {
			fmt::print("-- payer: {} {} ({})\n", payer.first_name,
			           payer.last_name, payer.tax_id);
			fmt::print("-- insured:\n");
			for (auto const& obj : insured) {
				auto const scale = obj.part_time_scale.value_or(ratio{});
				fmt::print("--   - {} {} ({}), {}/{} of {}\n", obj.first_name,
				           obj.last_name, obj.document, std::max(1u, scale.num),
				           std::max(1u, scale.den),
				           obj.salary
				               .transform([](auto const& value) {
					               return fmt::format("{} zł", value);
				               })
				               .value_or("<minimal pay>"));
			}
			return;
		} else {  // GCOV_EXCL_LINE[WIN32]
			fmt::print("-- payer:\n");
			fmt::print("--   name: {} {}\n", payer.first_name, payer.last_name);
			fmt::print("--   social id: {}\n", payer.social_id);
			fmt::print("--   tax id: {}\n", payer.tax_id);
			fmt::print("--   ident: {} {}\n", payer.kind, payer.document);

			fmt::print("-- insured:\n");
			for (auto const& obj : insured) {
				auto const scale = obj.part_time_scale.value_or(ratio{});

				fmt::print("--   - name: {} {}\n", obj.first_name,
				           obj.last_name);
				fmt::print("--     insurance title: {}\n",
				           fmt::join(obj.title.split(), " "));
				fmt::print("--     ident: {} {}\n", obj.kind, obj.document);
				fmt::print("--     salary: {}/{} of {}\n",
				           std::max(1u, scale.num), std::max(1u, scale.den),
				           obj.salary
				               .transform([](auto const& value) {
					               return fmt::format("{} zł", value);
				               })
				               .value_or("<minimal pay>"));
			}
		}

		if (level < verbose::parameters) {
			return;
		}

		fmt::print("-- parameters\n");
		fmt::print("--   cost of obtaining: {} zł / {} zł\n",
		           params.costs_of_obtaining.local,
		           params.costs_of_obtaining.remote);
		fmt::print("--   health: {}\n", from_rate(params.contributions.health));
		fmt::print("--   pension insurance: {}\n",
		           from_rate(params.contributions.pension_insurance));
		fmt::print("--   disability insurance: {}\n",
		           from_rate(params.contributions.disability_insurance));
		fmt::print("--   health insurance: {}\n",
		           from_rate(params.contributions.health_insurance));
		fmt::print("--   accident insurance: {}\n",
		           from_rate(params.contributions.accident_insurance));
		fmt::print("--   tax scale for month reported:\n");
		for (auto const& [amount, tax] : params.scale)
			fmt::print("--     over {} zł at {}%\n", amount, tax);
	}

	void tax_config::debug_print(verbose level) const noexcept {
		if (level < verbose::parameters) {
			return;
		}

		fmt::print("-- costs of obtaining per month:\n");
		for (auto const& [date, coo] : costs_of_obtaining) {
			fmt::print(
			    "--   {}-{:02}: {} zł / {} zł\n", static_cast<int>(date.year()),
			    static_cast<unsigned>(date.month()), coo.local, coo.remote);
		}

		fmt::print("-- minimal pay per month:\n");
		for (auto const& [date, amount] : minimal_pay) {
			fmt::print("--   {}-{:02}: {} zł\n", static_cast<int>(date.year()),
			           static_cast<unsigned>(date.month()), amount);
		}

		fmt::print("-- tax scale per month:\n");
		for (auto const& [date, levels] : scale) {
			fmt::print("--   {}-{:02}:\n", static_cast<int>(date.year()),
			           static_cast<unsigned>(date.month()));
			for (auto const& [amount, tax] : levels) {
				fmt::print("--     over {} zł at {}%\n", amount, tax);
			}
		}

		fmt::print("-- insurance rates per month:\n");
		for (auto const& [date, rates] : contributions) {
			fmt::print("--   {}-{:02}:\n", static_cast<int>(date.year()),
			           static_cast<unsigned>(date.month()));
			fmt::print("--     health: {}\n", from_rate(rates.health));
			fmt::print("--     pension insurance: {}\n",
			           from_rate(rates.pension_insurance));
			fmt::print("--     disability insurance: {}\n",
			           from_rate(rates.disability_insurance));
			fmt::print("--     health insurance: {}\n",
			           from_rate(rates.health_insurance));
			fmt::print("--     accident insurance: {}\n",
			           from_rate(rates.accident_insurance));
		}
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
