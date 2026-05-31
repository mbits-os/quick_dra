// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QFileSystemWatcher>
#include <QObject>
#include <app/utils/FormData.hpp>

namespace quick_dra::gui {
	class PageStack;

	class Globals : public QObject {
		Q_OBJECT

	public:
		Globals();

		static Globals* current();

		PageStack& stack() const noexcept { return *stack_; }
		bool hasStack() const noexcept { return !!stack_; }
		void setStack(PageStack* stack);

		bool configModified() const noexcept;

		void setConfig(std::filesystem::path const&, std::optional<std::filesystem::path> const&);
		FormData const& data() const noexcept { return data_; }
		ReportId const& reportId() const noexcept { return reportId_; }

		void reloadConfig();
		void storeIdentifier(ReportId const&);

	public slots:
		void setConfigModified(bool value);

	private slots:
		void observedFileChanged(QString const& path);

	signals:
		void configurationChanged();
		void identifierChanged();
		void formSetChanged();
		void configModifiedChanged(bool);

	private:
		void readSettings();
		void storeSettings();

		PageStack* stack_{};

		FormData data_{};
		ReportId reportId_{};
		QFileSystemWatcher watcher_{};
		bool configModified_{false};
	};
}  // namespace quick_dra::gui
