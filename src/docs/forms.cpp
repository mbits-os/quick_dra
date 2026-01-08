// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <array>
#include <chrono>
#include <concepts>
#include <quick_dra/docs/forms.hpp>
#include <string_view>

namespace quick_dra {
	namespace {
		using namespace std::chrono;

		template <std::integral T = int>
		T int_parse(std::string_view number) {
			T result{};
			auto const begin = number.data();
			auto const end = begin + number.size();
			auto const [ptr, ec] = std::from_chars(begin, end, result);
			if (ptr != end || ec != std::errc{}) {
				return 0;
			}
			return result;
		}

		year_month_day birthday_from_social_id(std::string_view social_id) {
			if (social_id.length() != 11) {
				return {};
			}

			auto const in_century = int_parse(social_id.substr(0, 2));
			auto const month_and_century =
			    int_parse<unsigned>(social_id.substr(2, 2));
			auto const day = int_parse(social_id.substr(4, 2));

			static constexpr auto const centuries =
			    std::array{1900, 2000, 2100, 2200, 1800};

			auto const month = static_cast<int>(month_and_century % 20);
			auto const century_code = month_and_century / 20u;
			auto const century = century_code >= centuries.size()
			                         ? 1900
			                         : centuries[century_code];

			return year{century + in_century} / month / day;
		}

		template <fixed_child Value>
		inline Value clamp(Value const& v) {
			return v.value < 0 ? Value{} : v;
		}

		template <fixed_child Value>
		inline Value round_to_whole(Value const& v) {
			auto const value =
			    ((v.value + (Value::den / 2)) / Value::den) * Value::den;
			return Value{value};
		}

		std::vector<calculated_value> compiled(insurance_title const& title) {
			auto parts = title.split();
			std::vector<calculated_value> result{};
			result.reserve(parts.size());
			for (auto& part : parts) {
				result.push_back(std::move(part));
			}
			return result;
		}

		currency get_currency(global_object const& data, varname const& var) {
			return data.typed_value<currency>(var);
		}

		void reduce_contribution(global_object& dst,
		                         global_object const& src,
		                         compiletime_varname path) {
			auto const var = varname::parse(path.name);
			auto& tgt = dst.get(var);
			auto const ptr = src.peek(var);
			if (!ptr) return;

			auto& from = *ptr;

			auto tgt_payer = get_currency(tgt, var::payer);
			auto tgt_insured = get_currency(tgt, var::insured);

			auto from_payer = get_currency(from, var::payer);
			auto from_insured = get_currency(from, var::insured);

			tgt = contribution{.payer = from_payer + tgt_payer,
			                   .insured = from_insured + tgt_insured};
		}

		void reduce_currency(global_object& dst,
		                     global_object const& src,
		                     compiletime_varname path) {
			auto const var = varname::parse(path.name);
			auto& tgt = dst.get(var);

			tgt = get_currency(tgt, {}) + get_currency(src, var);
		}

		void reduce_form(global_object& dst, global_object const& src) {
			reduce_contribution(dst, src, var::pension_insurance);
			reduce_contribution(dst, src, var::disability_insurance);
			reduce_contribution(dst, src, var::health_insurance);
			reduce_contribution(dst, src, var::accident_insurance);
			reduce_currency(dst, src, var::amount_payable);
		}

		form calc_common(std::string const& kedu,
		                 unsigned report_index,
		                 year_month const& date,
		                 year_month_day const& today,
		                 config const& cfg) {
			form result = {.key = kedu};
			auto& key = result.state.get(var::key);
			key.insert(var::NN, fmt::format("{:02}", report_index));
			key.insert(var::DATE,
			           fmt::format("{}-{:02}", static_cast<int>(date.year()),
			                       static_cast<unsigned>(date.month())));
			result.state.insert(
			    var::today,
			    fmt::format("{}-{:02}-{:02}", static_cast<int>(today.year()),
			                static_cast<unsigned>(today.month()),
			                static_cast<unsigned>(today.day())));

			auto& insurer = result.state.get(var::insurer);
			auto const& input = cfg.insurer;
			auto const bday = birthday_from_social_id(input.social_id);
			auto const _first = toupper_flt(input.first);
			auto const _last = toupper_flt(input.last);
			insurer.insert(var::tax_id, input.tax_id);
			insurer.insert(var::social_id, input.social_id);
			insurer.insert(var::document_kind, input.kind);
			insurer.insert(var::document, input.document);
			insurer.insert(var::name, fmt::format("{} {}", _first, _last));
			insurer.insert(var::last, _last);
			insurer.insert(var::first, _first);
			insurer.insert(
			    var::birthday,
			    fmt::format("{}-{:02}-{:02}", static_cast<int>(bday.year()),
			                static_cast<unsigned>(bday.month()),
			                static_cast<unsigned>(bday.day())));

			return result;
		}
	}  // namespace

	std::vector<calculated_section> form::fill(
	    bool verbose,
	    std::vector<compiled_section> const& tmplt) const {
		auto result = calculate(tmplt, state);

		if (verbose) {
			fmt::print("ZUS {}\n", key);
			debug_print(result);
		}

		return result;
	}

	form calc_rca(insured_t const& insured,
	              unsigned report_index,
	              year_month const& date,
	              year_month_day const& today,
	              config const& cfg) {
		auto const scale = insured.part_time_scale.value_or(ratio{});
		auto const scale_num = std::max(1u, scale.num);
		auto const scale_den = std::max(1u, scale.den);
		auto const salary =
		    insured.remuneration.value_or(cfg.params.minimal_pay);

		auto const baseline = [salary, scale_num, scale_den]() {
			auto const calc = salary.calc();
			return calc_currency{calc.value * scale_num / scale_den}.rounded();
		}();

		auto const tax_lowering_amount =
		    calc_currency{
		        (cfg.params.tax_free_allowance.calc() * cfg.params.tax_rate)
		            .value /
		        12}
		        .rounded();

		auto const health_insurance =
		    cfg.params.health_insurance.contribution_on(baseline);
		auto const pension_insurance =
		    cfg.params.pension_insurance.contribution_on(baseline);
		auto const disability_insurance =
		    cfg.params.disability_insurance.contribution_on(baseline);
		auto const accident_insurance =
		    cfg.params.accident_insurance.contribution_on(baseline);
		auto const guaranteed_employee_benefits_fund =
		    cfg.params.guaranteed_employee_benefits_fund.contribution_on(
		        baseline);

		auto const contributions = health_insurance.insured +
		                           pension_insurance.insured +
		                           disability_insurance.insured;
		auto const health_baseline = round_to_whole(
		    clamp(baseline - (contributions + cfg.params.cost_of_obtaining)));

		auto const advance =
		    (health_baseline.calc() * cfg.params.tax_rate).rounded();
		auto const tax = clamp(advance - cfg.params.free_amount);

		auto const health_lowered = clamp(advance - tax_lowering_amount);
		auto const health_contribution_intermediate =
		    (clamp(baseline - contributions).calc() * cfg.params.health)
		        .rounded();

		auto const health_contribution =
		    health_contribution_intermediate < health_lowered
		        ? health_contribution_intermediate
		        : health_lowered;

		auto const cost_of_insured = contributions + tax + health_contribution;
		auto const cost_of_payer =
		    health_insurance.payer + pension_insurance.payer +
		    disability_insurance.payer + accident_insurance.payer +
		    guaranteed_employee_benefits_fund.payer;

		auto result = calc_common("RCA"s, report_index, date, today, cfg);
		auto& person = result.state.get(var::insured);
		person.insert(var::document_kind, insured.kind);
		person.insert(var::document, insured.document);
		person.insert(var::last, toupper_flt(insured.last));
		person.insert(var::first, toupper_flt(insured.first));

		result.state.insert(var::insurance_title, compiled(insured.title));
		result.state.insert(var::scale.num, uint_value{scale_num});
		result.state.insert(var::scale.den, uint_value{scale_den});

		result.state.insert(var::remuneration.gross, baseline);
		result.state.insert(var::remuneration.net,
		                    clamp(baseline - cost_of_insured));
		result.state.insert(var::remuneration.payer_gross,
		                    baseline + cost_of_payer);

		result.state.insert(var::health_insurance, health_insurance);
		result.state.insert(var::pension_insurance, pension_insurance);
		result.state.insert(var::disability_insurance, disability_insurance);
		result.state.insert(var::accident_insurance, accident_insurance);
		result.state.insert(var::guaranteed_employee_benefits_fund,
		                    guaranteed_employee_benefits_fund);

		result.state.insert(var::health_baseline, health_baseline);
		result.state.insert(var::health_contribution, health_contribution);

		result.state.insert(
		    var::amount_payable,
		    health_insurance.total() + pension_insurance.total() +
		        disability_insurance.total() + accident_insurance.total());
		return result;
	}

	form calc_dra(unsigned report_index,
	              year_month const& date,
	              year_month_day const& today,
	              config const& cfg,
	              std::vector<form> const& forms) {
		auto result = calc_common("DRA"s, report_index, date, today, cfg);
		result.state.insert(
		    var::insured_count,
		    uint_value{static_cast<unsigned>(cfg.insured.size())});
		result.state.insert(var::accident_insurance_contribution,
		                    cfg.params.accident_insurance.total);

		for (auto const& src : forms) {
			if (src.key != "RCA"s) continue;
			reduce_form(result.state, src.state);
		}

		return result;
	}

	std::vector<form> prepare_form_set(bool verbose,
	                                   unsigned report_index,
	                                   std::chrono::year_month const& date,
	                                   std::chrono::year_month_day const& today,
	                                   config const& cfg) {
		std::vector<form> forms;
		forms.reserve(cfg.insured.size() + 1);

		for (auto const& insured : cfg.insured) {
			forms.push_back(calc_rca(insured, report_index, date, today, cfg));
		}

		forms.push_back(calc_dra(report_index, date, today, cfg, forms));
		if (verbose) {
			for (auto const& form : forms) {
				auto const doc_id =
				    forms.back().state.typed_value(var::insured.document, ""s);
				if (doc_id.empty()) {
					fmt::print("{}:", form.key);
				} else {
					fmt::print("{} [{}]:", form.key, doc_id);
				}
				form.state.debug_print(1);
			}
		}

		return forms;
	}
};  // namespace quick_dra