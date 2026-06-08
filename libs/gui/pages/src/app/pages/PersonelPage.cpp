// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <QMessageBox>
#include <QToolButton>
#include <app/controls/PageScrollArea.hpp>
#include <app/controls/PanelButtonGroup.hpp>
#include <app/gui/CurrentColor.hpp>
#include <app/gui/PageStack.hpp>
#include <app/pages/InsuredEditPage.hpp>
#include <app/pages/PayerEditPage.hpp>
#include <app/pages/PersonelPage.hpp>
#include <app/pages/RemoveInsuredPage.hpp>
#include <app/utils/LaidOut.hpp>
#include <app/utils/forms.hpp>
#include <app/utils/str.hpp>
#include <app/utils/utils.hpp>
#include <array>
#include <concepts>
#include <quick_dra/base/chrono.hpp>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/io/tax_config.hpp>
#include <quick_dra/models/project_reader.hpp>
#include <quick_dra/version.hpp>
#include <string_view>
#include <utility>

using namespace std::literals;

namespace quick_dra::gui {
	PersonelPage::PersonelPage(QWidget* parent) : PagedWidget(parent) { setupUI(); }

	PersonelPage::~PersonelPage() = default;

	void PersonelPage::setupUI() {
		setWindowTitle("Dane osobowe");
		auto const [_, pageParent] = PageScrollArea::setupPage(this);
		QVBoxLayout* pageLayout{};
		LaidOut{pageParent}.createLayout(pageLayout, "pageLayout", pageParent);
		auto page = LaidOut{pageParent, pageLayout};
		PanelButtonGroup* editInsuredGroup;
		page.createWidget(payerGroup, "payerGroup",
		                  [](PanelButtonGroup& group) { group.setSizePolicy(TakeWidth / HeightForWidth); })
		    .createWidget(editInsuredGroup, "editInsuredGroup",
		                  [](PanelButtonGroup& group) { group.setSizePolicy(TakeWidth / HeightForWidth); })
		    .createWidget(insuredGroup, "insuredGroup",
		                  [](PanelButtonGroup& group) { group.setSizePolicy(TakeWidth / HeightForWidth); });

		auto addNewInsuredButton = editInsuredGroup->createWidget<Panel>("addNewInsuredButton", [](Panel& panel) {
			panel.setInfo(QString::fromUtf8("Dodaj ubezpieczonego"), {}, {}, arrowRightSVGIcon());
		});
		addNewInsuredButton->setClickable(true);
		QObject::connect(addNewInsuredButton, &PanelButton::clicked, this, &PersonelPage::addInsured);

		removeInsuredButton = editInsuredGroup->createWidget<Panel>("removeInsuredButton", [](Panel& panel) {
			panel.setInfo(QString::fromUtf8("Usuń ubezpieczonych"), {}, {}, arrowRightSVGIcon());
		});
		removeInsuredButton->setClickable(true);
		removeInsuredButton->setEnabled(false);
		QObject::connect(removeInsuredButton, &PanelButton::clicked, this, &PersonelPage::removeInsured);
	}

	void PersonelPage::connectPage() {
		QObject::connect(&globals(), &Globals::identifierChanged, this, &PersonelPage::configurationChanged);
		QObject::connect(&globals(), &Globals::configurationChanged, this, &PersonelPage::configurationChanged);
		configurationChanged();
	}

	void PersonelPage::setPayer(partial::payer_t const& payer) {
		payerGroup->clearAll();

		auto const name = name_from(payer.first_name, payer.last_name, {.format_for = name_hint::payer});
		auto const info = second_line(document_info(payer.kind, payer.document), document_info('R', payer.tax_id),
		                              document_info('P', payer.social_id));
		auto const button = payerGroup->createWidget<Panel>("payerPanel", [&name, &info](Panel& panel) {
			panel.setInfo(QString::fromUtf8(name), QString::fromUtf8(info), {}, arrowRightSVGIcon());
		});
		button->setClickable(true);
		QObject::connect(button, &PanelButton::clicked, this, &PersonelPage::editPayer);
	}

	void PersonelPage::setInsured(std::vector<partial::insured_t> const& people) {
		removeInsuredButton->setEnabled(!people.empty());
		insuredGroup->clearAll();

		size_t index = 0;
		for (auto const& insured : people) {
			auto const name = name_from(insured.first_name, insured.last_name, {.format_for = name_hint::insured});
			auto const [employed, month, part_time_scale, salary] = insured.lookup(globals().reportId().date);
			auto const info =
			    employed ? second_line(document_info(insured.kind, insured.document),
			                           insurance_title_info(insured.title), salary_info(month, part_time_scale, salary))
			             : second_line(document_info(insured.kind, insured.document), "*poza okresem zatrudnienia*");
			auto const button = insuredGroup->createWidget<Panel>("payerPanel", [&name, &info](Panel& panel) {
				panel.setInfo(QString::fromUtf8(name), QString::fromUtf8(info), {}, arrowRightSVGIcon());
			});
			button->setClickable(true);
			QObject::connect(button, &PanelButton::clicked, [self = this, index]() { self->editInsured(index); });
			++index;
		}
	}

	void PersonelPage::configurationChanged() {
		auto const& formData = globals().data();
		if (formData.cfg.payer) {
			setPayer(formData.cfg.payer.value());
		} else {
			setPayer({});
		}
		if (formData.cfg.insured) {
			setInsured(formData.cfg.insured.value());
		} else {
			setInsured({});
		}
	}

	void PersonelPage::editPayer() { stack().push<PayerEditPage>(); }
	void PersonelPage::editInsured(size_t index) { stack().push<InsuredEditPage>(index); }
	void PersonelPage::addInsured() { stack().push<InsuredEditPage>(globals().data().cfg.insured->size()); }
	void PersonelPage::removeInsured() { stack().push<RemoveInsuredPage>(); }
}  // namespace quick_dra::gui
