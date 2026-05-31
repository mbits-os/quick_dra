// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <QToolButton>
#include <algorithm>
#include <app/controls/PageScrollArea.hpp>
#include <app/gui/CurrentColor.hpp>
#include <app/gui/PageStack.hpp>
#include <app/pages/HomePage.hpp>
#include <app/utils/LaidOut.hpp>
#include <app/utils/forms.hpp>
#include <app/utils/utils.hpp>
#include <array>
#include <concepts>
#include <format>
#include <quick_dra/base/chrono.hpp>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/base/str.hpp>
#include <quick_dra/io/tax_config.hpp>
#include <quick_dra/models/project_reader.hpp>
#include <quick_dra/version.hpp>
#include <string>
#include <string_view>
#include <utility>

using namespace std::literals;

namespace quick_dra::gui {
	namespace {
		std::string setFilename(unsigned report_index, year_month const& date) {
			return std::format("quick-dra_{}{:02}-{:02}.xml", static_cast<int>(date.year()),
			                   static_cast<unsigned>(date.month()), report_index);
		}
	}  // namespace

	HomePage::HomePage(QWidget* parent) : PagedWidget(parent) { setupUI(); }

	HomePage::~HomePage() = default;

	void HomePage::setupUI() {
		setWindowTitle("Podsumowanie");
		auto const [_, pageParent] = PageScrollArea::setupPage(this);

		QVBoxLayout* pageLayout{};
		LaidOut{pageParent}.createLayout(pageLayout, "pageLayout", pageParent);

		PanelButtonGroup* buttonGroup;
		auto page = LaidOut{pageParent, pageLayout};
		page.createWidget(
		        buttonGroup, "buttonGroup",
		        [self = this](PanelButtonGroup& group) {
			        group.setSizePolicy(TakeWidth / HeightForWidth);

			        auto identifierButton = group.createWidget<Panel>([](Panel& panel) {
				        panel.setInfo(QString::fromUtf8("Identyfikator"sv), {}, {}, arrowRightSVGIcon());
			        });

			        auto personelButton = group.createWidget<Panel>("personelButton", [](Panel& panel) {
				        panel.setInfo("Dane osobowe", {}, {}, arrowRightSVGIcon());
			        });
			        auto localStoreButton = group.createWidget<Panel>("localStoreButton", [](Panel& panel) {
				        panel.setInfo("Zapisz plik KEDU XML na dysku", {}, {}, ellipsisSVGIcon());
			        });
			        auto uploadKeduButton = group.createWidget<Panel>("localStoreButton", [](Panel& panel) {
				        panel.setInfo(QString::fromUtf16(u"Wyślij KEDU XML do e-ZUS"), {}, {}, ellipsisSVGIcon());
			        });

			        self->summaryIdentifier = static_cast<Panel*>(identifierButton->widget())->value();

			        identifierButton->setClickable(true);
			        identifierButton->setEnabled(false);
			        personelButton->setClickable(true);
			        personelButton->setEnabled(false);
			        localStoreButton->setClickable(true);
			        localStoreButton->setEnabled(false);
			        uploadKeduButton->setClickable(true);
			        uploadKeduButton->setEnabled(false);

			        QObject::connect(identifierButton, &PanelButton::clicked, self, &HomePage::editReportIdAction);
			        QObject::connect(personelButton, &PanelButton::clicked, self, &HomePage::showPersonelFilesAction);
			        QObject::connect(localStoreButton, &PanelButton::clicked, self, &HomePage::storeKeduXmlLocally);
		        })
		    .createWidget(summaryGroup, "summaryGroup",
		                  [](PanelButtonGroup& group) { group.setSizePolicy(TakeWidth / HeightForWidth); });
	}

	void HomePage::connectPage() {
		QObject::connect(&globals(), &Globals::formSetChanged, this, &HomePage::formSetChanged);
		reportIdChanged();
		formSetChanged();
	}

	void HomePage::editReportIdAction() {
		// push ReportIdEditPage
	}

	void HomePage::showPersonelFilesAction() {
		// push PersonelPage
	}

	void HomePage::storeKeduXmlLocally() {
		// globals().data().storeKedu(path);
	}

	void HomePage::reportIdChanged() { updateSummaryIdentifier(); }

	void HomePage::formSetChanged() {
		summaryGroup->clearAll();

		for (auto const& ref : globals().data().summary) {
			std::function<void()> slot{};
			if (ref.index != FormData::InvalidIndex) {
				slot = [self = this, index = ref.index] { self->pushFormView(index); };
			}
			auto button = layoutFormReference(summaryGroup, ref, slot);
			button->setEnabled(false);
		}
		updateSummaryIdentifier();
	}

	PanelButton* HomePage::layoutFormReference(PanelButtonGroup* group,
	                                           FormData::FormRef const& ref,
	                                           std::function<void()> const& slot) {
		auto button = group->createWidget<Panel>([&ref, hasSlot = !!slot](Panel& panel) {
			panel.setInfo(QString::fromUtf8(ref.label), QString::fromUtf8(ref.comment), QString::fromUtf8(ref.value),
			              hasSlot ? arrowRightSVGIcon() : QIcon{});
		});

		if (slot) {
			button->setClickable(true);
			QObject::connect(button, &PanelButton::clicked, [slot]() { slot(); });
		}

		return button;
	}

	void HomePage::updateSummaryIdentifier() {
		if (!summaryIdentifier) {
			return;
		}

		auto const& id = globals().reportId();
		auto const sid = std::format("{:02} {:02}-{:04}", id.index, static_cast<unsigned>(id.date.month()),
		                             static_cast<int>(id.date.year()));

		summaryIdentifier->setText(QString::fromStdString(sid));
	}

	void HomePage::pushFormView(size_t index) {
		// push ReportFormPage
	}
}  // namespace quick_dra::gui
