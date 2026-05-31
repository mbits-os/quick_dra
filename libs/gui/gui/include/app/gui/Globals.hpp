// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

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

		void setConfig(std::filesystem::path const&, std::optional<std::filesystem::path> const&);
		FormData const& data() const noexcept { return data_; }
		ReportId const& reportId() const noexcept { return reportId_; }

	signals:
		void configurationChanged();
		void formSetChanged();

	private:
		void readSettings();
		void storeSettings();

		PageStack* stack_{};

		FormData data_{};
		ReportId reportId_{};
	};
}  // namespace quick_dra::gui
