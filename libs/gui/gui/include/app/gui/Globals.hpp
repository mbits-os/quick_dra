// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QFileSystemWatcher>
#include <QObject>
#include <QSettings>
#include <app/gui/ShortcutDiscovery.hpp>
#include <app/utils/FormData.hpp>
#include <memory>
#include <utility>

namespace quick_dra::gui {
	class PageStack;

	class SettingsProvider {
	public:
		virtual ~SettingsProvider() = default;
		virtual QSettings createContainer() const = 0;

		template <typename T>
		static std::unique_ptr<SettingsProvider> wrap(T&& lambda) {
			struct Impl : SettingsProvider {
				T lambda;
				Impl(T&& lambda) : lambda{std::forward<T>(lambda)} {};
				QSettings createContainer() const override { return lambda(); }
			};

			return std::make_unique<Impl>(std::forward<T>(lambda));
		}

		static auto native() {
			return wrap([] { return QSettings{}; });
		}
	};

	class Globals : public QObject {
		Q_OBJECT

	public:
		Globals(std::unique_ptr<SettingsProvider> provider = {});

		static Globals* current();

		PageStack& stack() const noexcept { return *stack_; }
		bool hasStack() const noexcept { return !!stack_; }
		void setStack(PageStack* stack);

		ShortcutDiscovery& discovery() noexcept { return discovery_; }

		QSettings createSettings() const { return provider_->createContainer(); }

		bool configModified() const noexcept;

		void setConfig(std::filesystem::path const&,
		               std::optional<std::filesystem::path> const&,
		               bool download_github_config = true);
		FormData const& data() const noexcept { return data_; }
		ReportId const& reportId() const noexcept { return reportId_; }

		void reloadConfig();
		void storeIdentifier(ReportId const&);
		void storePayer(partial::payer_t const&);
		void storeInsured(size_t index, partial::insured_t const&);
		void removeInsured(std::vector<size_t> const& indexes);

	public slots:
		void setConfigModified(bool value);
		void observedFileChanged(QString const& path);
		void observedDirectoryChanged(QString const& path);

	signals:
		void configurationChanged();
		void identifierChanged();
		void formSetChanged();
		void configModifiedChanged(bool);

	private:
		void readSettings();
		void storeSettings();

		PageStack* stack_{};
		std::unique_ptr<SettingsProvider> provider_;

		FormData data_{};
		ReportId reportId_{};
		QFileSystemWatcher watcher_{};
		ShortcutDiscovery discovery_{};
		bool configModified_{false};
	};
}  // namespace quick_dra::gui
