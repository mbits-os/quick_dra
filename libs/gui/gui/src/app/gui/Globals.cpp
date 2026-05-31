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

		QObject::connect(&watcher_, &QFileSystemWatcher::fileChanged, this, &Globals::observedFileChanged);
	}

	Globals* Globals::current() { return globals; }

	void Globals::setStack(PageStack* stack) {
		stack_ = stack;
		stack->setGlobals(this);
	}

	void Globals::setConfig(std::filesystem::path const& cfg_path,
	                        std::optional<std::filesystem::path> const& tax_config_path) {
		if (!data_.cfg_path.empty()) {
			// TODO: remove previous?
		}
		auto canonical = std::filesystem::weakly_canonical(cfg_path);
		watcher_.addPath(QString::fromUtf8(as_sv(canonical.generic_u8string())));
		data_.setConfig(canonical, tax_config_path);
		configurationChanged();

		data_.lookupParameters(reportId_);
		formSetChanged();
	}

	void Globals::reloadConfig() {
		data_.loadConfig();
		data_.prepareFormData(reportId_);
		setConfigModified(false);
		formSetChanged();
		configurationChanged();
	}

	bool Globals::configModified() const noexcept { return configModified_; }
	void Globals::setConfigModified(bool value) {
		if (configModified_ == value) {
			return;
		}

		configModified_ = value;
		configModifiedChanged(configModified_);
	}

	void Globals::observedFileChanged(QString const& path) {
		auto const stdPath = std::filesystem::weakly_canonical(as_u8v(path.toUtf8()));
		if (stdPath == data_.cfg_path) {
			auto const access = std::filesystem::last_write_time(data_.cfg_path);
			if (access != data_.last_access) {
				setConfigModified(true);
			}
		}
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
