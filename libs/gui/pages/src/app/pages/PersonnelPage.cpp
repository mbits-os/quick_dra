// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <QMessageBox>
#include <QToolButton>
#include <app/controls/PageScrollArea.hpp>
#include <app/controls/Panel.hpp>
#include <app/controls/PanelButton.hpp>
#include <app/controls/PanelButtonGroup.hpp>
#include <app/gui/CurrentColor.hpp>
#include <app/gui/PageStack.hpp>
#include <app/pages/InsuredEditPage.hpp>
#include <app/pages/PayerEditPage.hpp>
#include <app/pages/PersonnelPage.hpp>
#include <app/pages/RemoveInsuredPage.hpp>
#include <app/utils/LaidOut.hpp>
#include <app/utils/forms.hpp>
#include <app/utils/str.hpp>
#include <app/utils/utils.hpp>
#include <array>
#include <cassert>
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
	QString operator"" _s(char16_t const* text, size_t length) {
		return QString{QStringView{text, static_cast<qsizetype>(length)}};
	}

	PersonnelPage::PersonnelPage(QWidget* parent) : PagedWidget(parent) { setupUI(); }

	PersonnelPage::~PersonnelPage() = default;

	void PersonnelPage::setupUI() {
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

		auto addNewInsuredButton =
		    editInsuredGroup->createPanel({.label = u"Dodaj ubezpieczonego"_s,
		                                   .toolTip = u"Dodaj ubezpieczonego"_s,
		                                   .rightIcon = arrowRightSVGIcon(),
		                                   .sequences = {Qt::CTRL | Qt::Key_A, Qt::CTRL | Qt::Key_D}},
		                                  "addNewInsuredButton");
		addNewInsuredButton->setClickable(true);
		QObject::connect(addNewInsuredButton, &PanelButton::clicked, this, &PersonnelPage::addInsured);

		removeInsuredButton = editInsuredGroup->createPanel({.label = u"Usuń ubezpieczonych"_s,
		                                                     .toolTip = u"Usuń ubezpieczonych"_s,
		                                                     .rightIcon = arrowRightSVGIcon(),
		                                                     .sequences = {Qt::CTRL | Qt::Key_U, Qt::CTRL | Qt::Key_R}},
		                                                    "removeInsuredButton");
		removeInsuredButton->setClickable(true);
		removeInsuredButton->setEnabled(false);
		QObject::connect(removeInsuredButton, &PanelButton::clicked, this, &PersonnelPage::removeInsured);
	}

	void PersonnelPage::connectPage() {
		QObject::connect(&globals(), &Globals::identifierChanged, this, &PersonnelPage::configurationChanged);
		QObject::connect(&globals(), &Globals::configurationChanged, this, &PersonnelPage::configurationChanged);
		configurationChanged();
	}

	void PersonnelPage::setPayer(partial::payer_t const& payer) {
		payerGroup->clearAll();

		auto const name = name_from(payer.first_name, payer.last_name, {.format_for = name_hint::payer});
		auto const info = second_line(document_info(payer.kind, payer.document), document_info('R', payer.tax_id),
		                              document_info('P', payer.social_id));
		auto const button = payerGroup->createPanel({.label = QString::fromUtf8(name),
		                                             .details = QString::fromUtf8(info),
		                                             .toolTip = u"Płatnik"_s,
		                                             .rightIcon = arrowRightSVGIcon(),
		                                             .sequences = {Qt::CTRL | Qt::Key_P}},
		                                            "payerPanel");
		button->setClickable(true);
		QObject::connect(button, &PanelButton::clicked, this, &PersonnelPage::editPayer);
	}

	void PersonnelPage::setInsured(std::vector<partial::insured_t> const& people) {
		removeInsuredButton->setEnabled(!people.empty());
		insuredGroup->clearAll();

		size_t index = 0;
		for (auto const& insured : people) {
			auto const name = name_from(insured.first_name, insured.last_name, {.format_for = name_hint::insured});
			auto const toolTip = std::format("Ubezpieczony nr {}", index + 1);
			auto const [employed, month, part_time_scale, salary] = insured.lookup(globals().reportId().date);
			auto const info =
			    employed ? second_line(document_info(insured.kind, insured.document),
			                           insurance_title_info(insured.title), salary_info(month, part_time_scale, salary))
			             : second_line(document_info(insured.kind, insured.document), "*poza okresem zatrudnienia*");
			static constexpr auto key1 = std::to_underlying(Qt::Key_1);
			auto const key = static_cast<Qt::Key>(key1 + index);
			auto const sequences = key <= Qt::Key_9 ? QList<QKeySequence>{Qt::CTRL | key} : QList<QKeySequence>{};
			auto const button = insuredGroup->createPanel({.label = QString::fromUtf8(name),
			                                               .details = QString::fromUtf8(info),
			                                               .toolTip = QString::fromUtf8(toolTip),
			                                               .rightIcon = arrowRightSVGIcon(),
			                                               .sequences = sequences},
			                                              "payerPanel");
			button->setClickable(true);
			QObject::connect(button, &PanelButton::clicked, [self = this, index]() { self->editInsured(index); });
			++index;
		}
	}

	void PersonnelPage::configurationChanged() {
		auto const& formData = globals().data();
		assert(formData.cfg.payer);
		assert(formData.cfg.insured);
		setPayer(formData.cfg.payer.value());
		setInsured(formData.cfg.insured.value());
	}

	void PersonnelPage::editPayer() { stack().push<PayerEditPage>(); }
	void PersonnelPage::editInsured(size_t index) { stack().push<InsuredEditPage>(index); }
	void PersonnelPage::addInsured() { stack().push<InsuredEditPage>(globals().data().cfg.insured->size()); }
	void PersonnelPage::removeInsured() { stack().push<RemoveInsuredPage>(); }
}  // namespace quick_dra::gui
