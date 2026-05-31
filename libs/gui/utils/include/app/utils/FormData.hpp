// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QVBoxLayout>
#include <QWidget>
#include <filesystem>
#include <limits>
#include <map>
#include <optional>
#include <quick_dra/docs/forms.hpp>
#include <quick_dra/docs/presentation.hpp>
#include <string>
#include <vector>
#include "LaidOut.hpp"

namespace quick_dra::gui {
	struct ReportId {
		unsigned index{1};
		year_month date{};
		bool isOverriden{};

		constexpr bool operator==(ReportId const&) const noexcept = default;
	};

	struct FormData {
		static constexpr auto InvalidIndex = std::numeric_limits<size_t>::max();

		struct FormRef {
			size_t index{InvalidIndex};
			std::string label{};
			std::string value{};
			std::string comment{};

			QWidget* createWidget(LaidOut<QWidget, QVBoxLayout> const&);
		};

		std::filesystem::path cfg_path{};
		partial::config cfg{};
		std::filesystem::file_time_type last_access{};
		std::optional<tax_config> tax_cfg{};
		tax_parameters tax_params{};
		compiled_templates templates{};
		std::vector<form> forms{};
		std::vector<FormRef> summary{};
		std::map<std::string, report_format::formatting> gui_formats{};

		void setConfig(std::filesystem::path const& cfg_path,
		               std::optional<std::filesystem::path> const& tax_config_path);
		void loadConfig();
		void storeConfig();
		void loadData();
		void lookupParameters(ReportId const& id);
		void prepareFormData(ReportId const& id);
		formatted_report formatReport(size_t index) const;
		void storeKedu(std::filesystem::path const& outputPath) const;
	};
}  // namespace quick_dra::gui
