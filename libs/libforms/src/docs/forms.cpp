// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <array>
#include <chrono>
#include <concepts>
#include <quick_dra/docs/forms.hpp>
#include <quick_dra/lex/tax.hpp>
#include <quick_dra/lex/validators.hpp>
#include <string_view>

namespace quick_dra {
	namespace {
		using namespace std::chrono;

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
			static auto const dummy = global_object{};
			auto const var = varname::parse(path.name);
			auto& tgt = dst.get(var);
			auto ptr = src.peek(var);
			if (!ptr) ptr = &dummy;

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
			reduce_currency(dst, src, var::insurance_total);
			reduce_currency(dst, src, var::tax_total);
		}

		form calc_common(std::string const& kedu,
		                 unsigned report_index,
		                 year_month const& date,
		                 year_month_day const& today,
		                 config const& cfg) {
			form result = {.key = kedu};
			auto& serial = result.state.get(var::serial);
			serial.insert(var::NN, fmt::format("{:02}", report_index));
			serial.insert(var::DATE,
			              fmt::format("{}-{:02}", static_cast<int>(date.year()),
			                          static_cast<unsigned>(date.month())));
			result.state.insert(
			    var::today,
			    fmt::format("{}-{:02}-{:02}", static_cast<int>(today.year()),
			                static_cast<unsigned>(today.month()),
			                static_cast<unsigned>(today.day())));

			auto& payer = result.state.get(var::payer);
			auto const& input = cfg.payer;
			auto const bday =
			    social_id_validator::get_birthday(input.social_id);
			auto const _first = to_upper(input.first_name);
			auto const _last = to_upper(input.last_name);
			payer.insert(var::tax_id, input.tax_id);
			payer.insert(var::social_id, input.social_id);
			payer.insert(var::document_kind, input.kind);
			payer.insert(var::document, input.document);
			payer.insert(var::name, fmt::format("{} {}", _first, _last));
			payer.insert(var::last, _last);
			payer.insert(var::first, _first);
			payer.insert(
			    var::birthday,
			    fmt::format("{}-{:02}-{:02}", static_cast<int>(bday.year()),
			                static_cast<unsigned>(bday.month()),
			                static_cast<unsigned>(bday.day())));

			return result;
		}
	}  // namespace

	std::vector<calculated_section> form::fill(
	    verbose level,
	    std::vector<compiled_section> const& tmplt) const {
		auto result = calculate(tmplt, state);

		if (level == verbose::calculated_sections) {
			auto doc_id = state.typed_value(var::insured.document, ""s);
			if (!doc_id.empty()) doc_id = fmt::format(" [{}]", doc_id);
			fmt::print("--   ZUS{}{}\n", key, doc_id);
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
		    calc_tax_lowering_amount(cfg.params.scale);

		auto const all_contributions =
		    cfg.params.contributions.contribution_on(baseline);

		auto const insured_contributions = all_contributions.insured();
		auto const taxed_baseline = round_to_whole(
		    clamp(baseline - (insured_contributions +
		                      cfg.params.costs_of_obtaining.local)));

		auto const tax_owed = calc_tax_owed(cfg.params.scale, taxed_baseline);

		auto const health_lowered = clamp(tax_owed - tax_lowering_amount);
		auto const health_contribution_intermediate =
		    (clamp(baseline - insured_contributions).calc() *
		     cfg.params.contributions.health.insured)
		        .rounded();

		auto const health_contribution =
		    health_contribution_intermediate < health_lowered
		        ? health_contribution_intermediate
		        : health_lowered;

		auto const cost_on_insured =
		    insured_contributions + tax_owed + health_contribution;
		auto const cost_on_payer = all_contributions.payer();

		auto result = calc_common("RCA"s, report_index, date, today, cfg);
		auto& person = result.state.get(var::insured);
		person.insert(var::document_kind, insured.kind);
		person.insert(var::document, insured.document);
		person.insert(var::last, to_upper(insured.last_name));
		person.insert(var::first, to_upper(insured.first_name));

		result.state.insert(var::insurance_title, compiled(insured.title));
		result.state.insert(var::scale.num, uint_value{scale_num});
		result.state.insert(var::scale.den, uint_value{scale_den});

		result.state.insert(var::remuneration.gross, baseline);
		result.state.insert(var::remuneration.net,
		                    clamp(baseline - cost_on_insured));
		result.state.insert(var::remuneration.payer_gross,
		                    baseline + cost_on_payer);

		result.state.insert(var::health_insurance,
		                    all_contributions.health_insurance);
		result.state.insert(var::pension_insurance,
		                    all_contributions.pension_insurance);
		result.state.insert(var::disability_insurance,
		                    all_contributions.disability_insurance);
		result.state.insert(var::accident_insurance,
		                    all_contributions.accident_insurance);
		result.state.insert(var::guaranteed_employee_benefits_fund,
		                    contribution{});

		result.state.insert(var::health_baseline, taxed_baseline);
		result.state.insert(var::health_contribution, health_contribution);

		result.state.insert(var::insurance_total, all_contributions.total());
		result.state.insert(var::tax_total, tax_owed);
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
		result.state.insert(
		    var::accident_insurance_contribution,
		    cfg.params.contributions.accident_insurance.total());

		reduce_form(result.state, {});
		for (auto const& src : forms) {
			if (src.key != "RCA"s) continue;
			reduce_form(result.state, src.state);
		}

		return result;
	}

	std::vector<form> prepare_form_set(verbose level,
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
		if (level >= verbose::raw_form_data) {
			fmt::print("-- form data:\n");
			for (auto const& form : forms) {
				auto const doc_id =
				    form.state.typed_value(var::insured.document, ""s);
				if (doc_id.empty()) {
					fmt::print("--   {}:", form.key);
				} else {
					fmt::print("--   {} [{}]:", form.key, doc_id);
				}
				form.state.debug_print(2);
				fmt::print("--\n");
			}
		}

		return forms;
	}
};  // namespace quick_dra
