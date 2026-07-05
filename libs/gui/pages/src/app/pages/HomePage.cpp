// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QToolButton>
#include <algorithm>
#include <app/controls/PageScrollArea.hpp>
#include <app/controls/Panel.hpp>
#include <app/controls/PanelButton.hpp>
#include <app/gui/CurrentColor.hpp>
#include <app/gui/PageStack.hpp>
#include <app/pages/HomePage.hpp>
#include <app/pages/PersonnelPage.hpp>
#include <app/pages/ReportFormPage.hpp>
#include <app/pages/ReportIdEditPage.hpp>
#include <app/utils/LaidOut.hpp>
#include <app/utils/forms.hpp>
#include <app/utils/str.hpp>
#include <app/utils/utils.hpp>
#include <array>
#include <concepts>
#include <format>
#include <quick_dra/base/chrono.hpp>
#include <quick_dra/base/paths.hpp>
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

		QString standardWritePath() {
			QString directory{};
			for (auto const loc : {
			         QStandardPaths::DocumentsLocation,
			         QStandardPaths::DownloadLocation,
			         QStandardPaths::DesktopLocation,
			     }) {
				if (!directory.isEmpty()) {
					break;
				}
				directory = QStandardPaths::writableLocation(loc);
			}
			return directory;
		}  // GCOV_EXCL_LINE[GCC]

		QString previousSaveDirectory(Globals& globals) {
			auto settings = globals.createSettings();
			settings.beginGroup("State");
			auto const dirname = settings.value("SaveDirectory", "").toString();
			settings.endGroup();
			return !dirname.isEmpty() && QDir{dirname}.exists() ? dirname : QString{};
		}

		void storeSaveDirectory(Globals& globals, QString const& dirname) {
			auto settings = globals.createSettings();
			settings.beginGroup("State");
			settings.setValue("SaveDirectory", dirname);
			settings.endGroup();
		}

		QString writePath(Globals& globals) {
			auto result = previousSaveDirectory(globals);
			if (result.isEmpty()) result = standardWritePath();
			return result;
		}  // GCOV_EXCL_LINE[GCC]
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

			        auto identifierButton =
			            group.createPanel({.label = "Identyfikator", .rightIcon = arrowRightSVGIcon()});

			        auto personelButton = group.createPanel({.label = "Dane osobowe", .rightIcon = arrowRightSVGIcon()},
			                                                "personelButton");
			        auto localStoreButton = group.createPanel(
			            {.label = "Zapisz plik KEDU XML na dysku", .rightIcon = ellipsisSVGIcon()}, "localStoreButton");
			        group.createPanel({.label = QString::fromUtf16(u"Wyślij KEDU XML do e-ZUS"),
			                           .rightIcon = ellipsisSVGIcon(),
			                           .isEnabled = false},
			                          "localStoreButton");

			        self->identifierPanel = static_cast<Panel*>(identifierButton->widget());

			        QObject::connect(identifierButton, &PanelButton::clicked, self, &HomePage::editReportIdAction);
			        QObject::connect(personelButton, &PanelButton::clicked, self, &HomePage::showPersonelFilesAction);
			        QObject::connect(localStoreButton, &PanelButton::clicked, self, &HomePage::storeKeduXmlLocally);
		        })
		    .createWidget(summaryGroup, "summaryGroup",
		                  [](PanelButtonGroup& group) { group.setSizePolicy(TakeWidth / HeightForWidth); });
	}

	void HomePage::connectPage() {
		QObject::connect(&globals(), &Globals::identifierChanged, this, &HomePage::reportIdChanged);
		QObject::connect(&globals(), &Globals::formSetChanged, this, &HomePage::formSetChanged);
		reportIdChanged();
		formSetChanged();
	}

	void HomePage::editReportIdAction() {
		auto const dlg = push<ReportIdEditPage>();
		dlg->initialData(globals().reportId());
		QObject::connect(dlg, &ReportIdEditPage::identifierUpdated, this, &HomePage::reportIdAccepted);
	}

	void HomePage::showPersonelFilesAction() { push<PersonnelPage>(); }

	void HomePage::storeKeduXmlLocally() {
		auto const& id = globals().reportId();
		auto const input_path = QDir{writePath(globals())}.filePath(QString::fromUtf8(setFilename(id.index, id.date)));
		auto const output_path =
		    property("kedu_path").isNull()
		        ? QFileDialog::getSaveFileName(this, "Zapisz plik", input_path, "XML Files (*.xml)")
		        : property("kedu_path").toString();

		if (output_path.isEmpty()) {
			return;  // GCOV_EXCL_LINE -- hard to test for not writing a file we have no name for
		}

		auto const path = std::filesystem::absolute(QFile{output_path}.filesystemFileName());
		storeSaveDirectory(globals(), QString::fromUtf8(as_sv(path.parent_path().generic_u8string())));
		globals().data().storeKedu(path);
	}

	void HomePage::reportIdAccepted(int serial, QDate const& date, bool moved) {
		auto const index = static_cast<unsigned>(std::min(std::max(serial, 1), 99));
		globals().storeIdentifier({
		    .index = index,
		    .date = year{date.year()} / date.month(),
		    .isOverriden = moved,
		});
	}

	void HomePage::reportIdChanged() { updateIdentifierPanel(); }

	void HomePage::formSetChanged() {
		summaryGroup->clearAll();

		for (auto const& ref : globals().data().summary) {
			std::function<void()> slot{};
			if (ref.index != FormData::InvalidIndex) {
				slot = [self = this, index = ref.index] { self->pushFormView(index); };
			}
			layoutFormReference(summaryGroup, ref, slot);
		}
		updateIdentifierPanel();
	}

	PanelButton* HomePage::layoutFormReference(PanelButtonGroup* group,
	                                           FormData::FormRef const& ref,
	                                           std::function<void()> const& slot) {
		auto const hasSlot = !!slot;
		auto button = group->createPanel({
		    .label = QString::fromUtf8(ref.label),
		    .details = ref.comment.empty() ? QString() : QString::fromUtf8(ref.comment),
		    .value = ref.value.empty() ? QString() : QString::fromUtf8(ref.value),
		    .rightIcon = hasSlot ? arrowRightSVGIcon() : QIcon{},
		    .isClickable = hasSlot,
		});

		if (hasSlot) {
			QObject::connect(button, &PanelButton::clicked, [slot]() { slot(); });
		}

		return button;
	}

	void HomePage::updateIdentifierPanel() {
		if (!identifierPanel) {
			return;  // GCOV_EXCL_LINE
		}

		auto const& id = globals().reportId();
		auto const sid = std::format("{:02} {:02}-{:04}", id.index, static_cast<unsigned>(id.date.month()),
		                             static_cast<int>(id.date.year()));

		identifierPanel->setValue(QString::fromStdString(sid));
	}

	void HomePage::pushFormView(size_t index) { stack().push<ReportFormPage>(index); }
}  // namespace quick_dra::gui
