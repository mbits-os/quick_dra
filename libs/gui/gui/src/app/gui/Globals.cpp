// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QSettings>
#include <app/gui/Globals.hpp>
#include <app/gui/PageStack.hpp>
#include <quick_dra/models/project_reader.hpp>

namespace quick_dra::gui {
	namespace {
		Globals* globals{};
	}  // namespace

	Globals::Globals() {
		globals = this;
		data_.loadData();
		readSettings();
	}

	Globals* Globals::current() { return globals; }

	void Globals::setStack(PageStack* stack) {
		stack_ = stack;
		stack->setGlobals(this);
	}

	void Globals::setConfig(std::filesystem::path const& cfg_path,
	                        std::optional<std::filesystem::path> const& tax_config_path) {
		data_.setConfig(cfg_path, tax_config_path);
		configurationChanged();

		data_.lookupParameters(reportId_);
		formSetChanged();
	}

	void Globals::readSettings() {
		QSettings settings{};
		settings.beginGroup("Settings");
		reportId_.isOverriden =
		    yaml::convert_string(settings.value("YearMonth", "").toString().toStdString(), reportId_.date);
		if (!reportId_.isOverriden) {
			auto const today = get_today();
			reportId_.date = today.year() / today.month() - months{1};
		}
		reportId_.index = settings.value("ReportIndex", 1).toUInt();
		settings.endGroup();
	}

	void Globals::storeSettings() {
		QSettings settings{};
		settings.beginGroup("Settings");
		if (reportId_.isOverriden) {
			settings.setValue("YearMonth", QString::fromUtf8(yaml::as_string(reportId_.date)));
		} else {
			settings.remove("YearMonth");
		}
		settings.setValue("ReportIndex", reportId_.index);
		settings.endGroup();
	}
}  // namespace quick_dra::gui
