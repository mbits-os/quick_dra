// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QFile>
#include <app/utils/FormData.hpp>
#include <app/utils/forms.hpp>
#include <format>
#include <fstream>
#include <map>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/docs/file_set.hpp>
#include <quick_dra/docs/locale.hpp>
#include <quick_dra/docs/xml.hpp>
#include <quick_dra/docs/xml_builder.hpp>
#include <quick_dra/io/tax_config.hpp>
#include <string>
#include <vector>

namespace quick_dra::gui {
	namespace {
		person personFrom(partial::person const& p) {
			return person{
			    .last_name = p.last_name.value_or("??"s),
			    .id_card = p.id_card.value_or(""s),
			    .passport = p.passport.value_or(""s),
			    .first_name = p.first_name.value_or("??"s),
			    .kind = p.kind.value_or("?"s),
			    .document = p.document.value_or("?????"s),
			};
		}

		payer_t payerFrom(partial::payer_t const& payer) {
			return payer_t{
			    personFrom(payer),
			    payer.tax_id.value_or(""s),
			    payer.social_id.value_or(""s),
			};
		}

		employment_history historyFrom(partial::employment_history const& employment) {
			return employment_history{
			    .part_time_scale = employment.part_time_scale,
			    .salary = employment.salary,
			};
		}

		std::map<year_month, employment_history> historyFrom(
		    std::map<year_month, partial::employment_history> const& history) {
			std::map<year_month, employment_history> result{};

			for (auto const& [key, item] : history) {
				result[key] = historyFrom(item);
			}

			return result;
		}

		insured_t insuredFrom(partial::insured_t const& insured) {
			return insured_t{
			    personFrom(insured),
			    insured.title ? *insured.title : insurance_title{"9999"s, 9, 9},
			    insured.social_id.value_or(""s),
			    historyFrom(insured.history.value_or<std::map<year_month, partial::employment_history>>({})),
			};
		}

		std::vector<insured_t> insuredFrom(std::vector<partial::insured_t> const& insured) {
			std::vector<insured_t> result{};
			result.reserve(insured.size());

			for (auto const& item : insured) {
				result.emplace_back(insuredFrom(item));
			}

			return result;
		}

		config configFrom(partial::config const& cfg, quick_dra::v1::tax_parameters const& params) {
			return {
			    .version = cfg.version.value_or(kApiVersion),
			    .payer = payerFrom(cfg.payer.value_or<partial::payer_t>({})),
			    .insured = insuredFrom(cfg.insured.value_or<std::vector<partial::insured_t>>({})),
			    .accident_insurance = {},
			    .params = params,
			};
		}
		std::string currency_info(std::string_view label, currency value) {
			return info_span(label, locale::from_system(value));
		}
	}  // namespace

	void FormData::setConfig(std::filesystem::path const& path,
	                         std::optional<std::filesystem::path> const& tax_config_path) {
		cfg_path = path;
		loadConfig();
		tax_cfg = load_tax_config(verbose::none, tax_config_path);
	}

	void FormData::loadConfig() {
		last_access = std::filesystem::last_write_time(cfg_path);
		cfg = partial::config::load_partial(cfg_path, false);
		if (!cfg.insured) cfg.insured.emplace();
		if (!cfg.payer) cfg.payer.emplace();
	}

	void FormData::storeConfig() {
		cfg.store(cfg_path);
		last_access = std::filesystem::last_write_time(cfg_path);
		cfg.postprocess();
		if (cfg.payer) cfg.payer->postprocess();
		if (cfg.insured) {
			for (auto& insured : *cfg.insured) {
				insured.postprocess();
			}
		}
	}

	void FormData::loadData() {
		templates.reports.clear();
		gui_formats.clear();

		if (auto raw_templates = quick_dra::templates::parse_yaml(platform::config_data_dir() / "templates.yaml"sv);
		    raw_templates) {
			templates = compiled_templates::compile(*raw_templates);
		}

		if (auto file = QFile{":/report_format.yaml"}; file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			gui_formats = report_format::formatting::parse(file.readAll().toStdString(), ":/report_format.yaml"s);
		}
	}

	void FormData::lookupParameters(ReportId const& id) {
		tax_params = {};
		if (tax_cfg) {
			lookup_parameters(tax_params, *tax_cfg, cfg.accident_insurance, id.date);
		}
		prepareFormData(id);
	}

	void FormData::prepareFormData(ReportId const& id) {
		forms = prepare_form_set(verbose::none, id.index, id.date, get_today(), configFrom(cfg, tax_params));
		summary.clear();

		size_t count = 0;
		for (auto const& form : forms) {
			if (form.key == "RCA"sv)
				++count;
			else if (form.key == "DRA"sv)
				count += 2;
		}
		summary.reserve(count);

		size_t index = 0;
		for (auto const& form : forms) {
			++index;
			if (form.key != "RCA"sv) continue;
			auto const first_name = form.state.typed_value(var::insured.first, "??"s);
			auto const last_name = form.state.typed_value(var::insured.last, "??"s);
			auto const net_salary = form.state.typed_value(var::salary.net, 0_PLN);
			auto const health_contribution = form.state.typed_value(var::health_contribution, 0_PLN);
			auto const tax_total = form.state.typed_value(var::tax_total, 0_PLN);
			auto const insurance_total = form.state.typed_value(var::insurance_total, 0_PLN);
			summary.push_back({
			    .index = index - 1,
			    .label = std::format("{}, {}", last_name, first_name),
			    .value = locale::from_system(net_salary),
			    .comment =
			        second_line(currency_info("społeczne"sv, insurance_total - health_contribution),
			                    currency_info("zdrowotne", health_contribution), currency_info("podatek", tax_total)),
			});
		}

		index = 0;
		for (auto const& form : forms) {
			++index;
			if (form.key != "DRA"sv) continue;
			auto const tax_total = form.state.typed_value(var::tax_total, 0_PLN);
			auto const insurance_total = form.state.typed_value(var::insurance_total, 0_PLN);
			summary.push_back({
			    .index = index - 1,
			    .label = "Dla ZUS"s,
			    .value = locale::from_system(insurance_total),
			});
			summary.push_back({
			    .index = InvalidIndex,
			    .label = "Dla Urzędu Skarbowego"s,
			    .value = locale::from_system(tax_total),
			});
		}
	}

	formatted_report FormData::formatReport(size_t index) const {
		if (index >= forms.size()) return {.title = "! <internal error>"s};
		auto const& form_data = forms.at(index);

		auto it = templates.reports.find(form_data.key);
		if (it == templates.reports.end()) {
			// TODO: error scenario
			return {.title = std::format("! {} <internal error>", form_data.key)};
		}

		auto const filled = form_data.fill(verbose::none, it->second);
		return report_format::formatting::format_report(gui_formats, form_data.key, filled);
	}

	void FormData::storeKedu(std::filesystem::path const& filename) const {
		auto const tree = build_file_set(verbose::none, forms, templates);
		auto file = std::ofstream{filename};
		file << tree;
	}
}  // namespace quick_dra::gui
