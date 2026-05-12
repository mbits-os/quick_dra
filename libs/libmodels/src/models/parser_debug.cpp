// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/ranges.h>
#include <algorithm>
#include <quick_dra/base/str.hpp>
#include <quick_dra/models/types.hpp>
#include <string>
#include <vector>

namespace quick_dra {
	namespace {
		std::string from_rate(rate const& r) {
			std::vector<std::string> result{};
			result.reserve(2);
			if (r.payer != 0_per) {
				result.emplace_back(fmt::format("payer {}%", r.payer));
			}
			if (r.insured != 0_per) {
				result.emplace_back(fmt::format("insured {}%", r.insured));
			}
			// GCOV_EXCL_START
			if (result.empty()) {
				[[unlikely]];
				result.emplace_back("payer 0%"s);
			}
			// GCOV_EXCL_STOP
			return join(result, ", "_sep);  //-V601
		}

		template <typename Insured>
		void names_only(payer_t const& payer, std::vector<Insured> const& insured) {
			fmt::print("-- payer: {} {}\n", payer.first_name, payer.last_name);
			fmt::print("-- insured:\n");
			for (auto const& obj : insured) {
				fmt::print("--   - {} {}\n", obj.first_name, obj.last_name);
			}
		}

		void names_summary_and_beyond(payer_t const& payer, verbose level) {
			if (level == verbose::names_and_summary) {
				fmt::print("-- payer: {} {} ({})\n", payer.first_name, payer.last_name, payer.tax_id);
				return;
			}  // GCOV_EXCL_LINE[WIN32]

			fmt::print("-- payer:\n");
			fmt::print("--   name: {} {}\n", payer.first_name, payer.last_name);
			fmt::print("--   social id: {}\n", payer.social_id);
			fmt::print("--   tax id: {}\n", payer.tax_id);
			fmt::print("--   ident: {} {}\n", payer.kind, payer.document);
		}

		void names_summary_and_beyond(std::vector<v2::insured_t> const& insured, verbose level) {
			if (level == verbose::names_and_summary) {
				fmt::print("-- insured:\n");
				for (auto const& obj : insured) {
					std::vector<std::string> history{};
					history.reserve(obj.history.size());
					for (auto const& [key, value] : obj.history) {
						auto const scale = value.part_time_scale.value_or(ratio{});
						auto const scale_str = fmt::format("{}/{}", std::max(1u, scale.num), std::max(1u, scale.den));
						auto const salary_str =
						    value.salary.transform([](auto const& value) { return fmt::format("{} zł", value); })
						        .value_or("<minimal pay>");
						if (key == null_month) {
							history.push_back(fmt::format("{} of {}", scale_str, salary_str));
						} else {
							history.push_back(fmt::format("{} of {} [{}]", scale_str, salary_str, fmt_date_slash(key)));
						}
					}
					fmt::print("--   - {} {} ({}), {}\n", obj.first_name, obj.last_name, obj.document,
					           fmt::join(history, "; "));
				}
				return;
			}  // GCOV_EXCL_LINE[WIN32]

			fmt::print("-- insured:\n");
			for (auto const& obj : insured) {
				std::vector<std::string> history{};
				history.reserve(obj.history.size());
				for (auto const& [key, value] : obj.history) {
					auto const scale = value.part_time_scale.value_or(ratio{});
					auto const scale_str = fmt::format("{}/{}", std::max(1u, scale.num), std::max(1u, scale.den));
					auto const salary_str =
					    value.salary.transform([](auto const& value) { return fmt::format("{} zł", value); })
					        .value_or("<minimal pay>");
					if (key == null_month) {
						history.push_back(fmt::format("{} of {}", scale_str, salary_str));
					} else {
						history.push_back(fmt::format("{} of {} [{}]", scale_str, salary_str, fmt_date_slash(key)));
					}
				}

				fmt::print("--   - name: {} {}\n", obj.first_name, obj.last_name);
				fmt::print("--     insurance title: {}\n", fmt::join(obj.title.split(), " "));
				fmt::print("--     ident: {} {}\n", obj.kind, obj.document);
				fmt::print("--     salary: {}\n", fmt::join(history, "\n               "));
			}
		}

		void parameters(tax_parameters const& params) {
			fmt::print("-- parameters\n");
			fmt::print("--   cost of obtaining: {} zł / {} zł\n",  // GCOV_EXCL_LINE
			           params.costs_of_obtaining.local, params.costs_of_obtaining.remote);
			fmt::print("--   health: {}\n", from_rate(params.contributions.health));
			fmt::print("--   pension insurance: {}\n", from_rate(params.contributions.pension_insurance));
			fmt::print("--   disability insurance: {}\n", from_rate(params.contributions.disability_insurance));
			fmt::print("--   health insurance: {}\n", from_rate(params.contributions.health_insurance));
			fmt::print("--   accident insurance: {}\n", from_rate(params.contributions.accident_insurance));
			fmt::print("--   tax scale for month reported:\n");
			for (auto const& [amount, tax] : params.scale)
				fmt::print("--     over {} zł at {}%\n", amount, tax);
		}
	}  // namespace
}  // namespace quick_dra

namespace quick_dra::v1 {
	void tax_config::debug_print(verbose level) const noexcept {
		if (level < verbose::parameters) {
			return;
		}

		fmt::print("-- costs of obtaining per month:\n");
		for (auto const& [date, coo] : costs_of_obtaining) {
			fmt::print("--   {}-{:02}: {} zł / {} zł\n", static_cast<int>(date.year()),
			           static_cast<unsigned>(date.month()), coo.local, coo.remote);
		}

		fmt::print("-- minimal pay per month:\n");
		for (auto const& [date, amount] : minimal_pay) {
			fmt::print("--   {}-{:02}: {} zł\n", static_cast<int>(date.year()), static_cast<unsigned>(date.month()),
			           amount);
		}

		fmt::print("-- tax scale per month:\n");
		for (auto const& [date, levels] : scale) {
			fmt::print("--   {}-{:02}:\n", static_cast<int>(date.year()), static_cast<unsigned>(date.month()));
			for (auto const& [amount, tax] : levels) {
				fmt::print("--     over {} zł at {}%\n", amount, tax);
			}
		}

		fmt::print("-- insurance rates per month:\n");
		for (auto const& [date, rates] : contributions) {
			fmt::print("--   {}-{:02}:\n", static_cast<int>(date.year()), static_cast<unsigned>(date.month()));
			fmt::print("--     health: {}\n", from_rate(rates.health));
			fmt::print("--     pension insurance: {}\n", from_rate(rates.pension_insurance));
			fmt::print("--     disability insurance: {}\n", from_rate(rates.disability_insurance));
			fmt::print("--     health insurance: {}\n", from_rate(rates.health_insurance));
			fmt::print("--     accident insurance: {}\n", from_rate(rates.accident_insurance));
		}
	}
}  // namespace quick_dra::v1

namespace quick_dra::v2 {
	void config::debug_print(verbose level) const noexcept {
		if (level == verbose::none) {
			return;
		}

		if (level == verbose::names_only) {
			names_only(payer, insured);
		} else {  // GCOV_EXCL_LINE[WIN32]
			names_summary_and_beyond(payer, level);
			names_summary_and_beyond(insured, level);
		}

		if (level < verbose::parameters) {
			return;
		}

		parameters(params);
	}
}  // namespace quick_dra::v2

namespace quick_dra {
	void compiled_templates::debug_print() const noexcept {
		fmt::print("-- templates:\n");

		for (auto const& [kedu, report] : reports) {
			fmt::print("--   [{}]:\n", kedu);
			quick_dra::debug_print(report);
		}
	}
}  // namespace quick_dra
