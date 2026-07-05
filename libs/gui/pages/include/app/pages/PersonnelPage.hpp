// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QFont>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QList>
#include <QSlider>
#include <QWidget>
#include <app/controls/PanelButtonGroup.hpp>
#include <app/gui/PagedWidget.hpp>
#include <app/utils/FormData.hpp>
#include <quick_dra/docs/forms.hpp>
#include <vector>

#undef emit
#include <quick_dra/models/types.hpp>

namespace quick_dra::gui {
	class PersonnelPage : public PagedWidget {
		Q_OBJECT

	public:
		explicit PersonnelPage(QWidget* parent = nullptr);
		~PersonnelPage();

		void connectPage() override;
		bool survivesReload() const override { return true; }
		PanelButtonGroup* insuredButtons() const { return insuredGroup; }

	public slots:
		void configurationChanged();
		void editPayer();
		void addInsured();
		void removeInsured();
		void editInsured(size_t);

	private:
		void setupUI();

		void setPayer(partial::payer_t const&);
		void setInsured(std::vector<partial::insured_t> const&);

		PanelButtonGroup* payerGroup{};
		PanelButtonGroup* insuredGroup{};
		PanelButton* removeInsuredButton{};
	};
}  // namespace quick_dra::gui
