// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QStandardItemModel>
#include <app/controls/PageScrollArea.hpp>
#include <app/controls/PanelButtonGroup.hpp>
#include <app/gui/CurrentColor.hpp>
#include <app/gui/PageStack.hpp>
#include <app/pages/InsuredEditPage.hpp>
#include <app/pages/PayerEditPage.hpp>
#include <app/pages/RemoveInsuredPage.hpp>
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
#include <vector>

using namespace std::literals;

namespace quick_dra::gui {
	RemoveInsuredPage::RemoveInsuredPage(QWidget* parent) : PagedWidget(parent) { setupUI(); }

	RemoveInsuredPage::~RemoveInsuredPage() = default;

	void RemoveInsuredPage::setupUI() {
		setWindowTitle("Wybierz pozycje do usunięcia");
		auto const [_, pageParent] = PageScrollArea::setupPage(this);

		QVBoxLayout* mainLayout{};

		LaidOut{pageParent}.createLayout(mainLayout, "",
		                                 pageParent);  //, [](auto& layout) { layout.setContentsMargins(0, 0, 0, 0); });
		LaidOut{pageParent, mainLayout}
		    .createWidget(selectAll, "selectAll",
		                  [](auto& wgt) {
			                  wgt.setSizePolicy(TakeWidth);
			                  wgt.setText("Wybierz wszystko");
			                  // wgt.setTristate();
		                  })
		    .createWidget(insuredList, "insuredList", [](auto& wgt) {
			    wgt.setSizePolicy(TakeWidth);
			    wgt.setIndentation(0);
		    });

		QObject::connect(selectAll, &QCheckBox::checkStateChanged, this, &RemoveInsuredPage::selectAllStateChanged);
		QObject::connect(insuredList, &QTreeView::activated, this, &RemoveInsuredPage::swapCheckState);
	}

	void RemoveInsuredPage::connectPage() {
		QObject::connect(&globals(), &Globals::configurationChanged, this, &RemoveInsuredPage::configurationChanged);
		configurationChanged();
	}

	void RemoveInsuredPage::accept() {
		auto const model = qobject_cast<QStandardItemModel*>(insuredList->model());
		if (model) {
			std::vector<size_t> removed{};
			auto const rows = model->rowCount();
			removed.reserve(static_cast<size_t>(rows));
			for (int rowIndex = 0; rowIndex < rows; ++rowIndex) {
				auto const item = model->item(rowIndex);
				if (!item) continue;
				if (item->checkState() == Qt::Checked) {
					removed.push_back(static_cast<size_t>(rowIndex));
				}
			}

			if (!removed.empty()) {
				globals().removeInsured(removed);
			}
		}

		selectAll->setChecked(false);
		setFormDirty(false);
		leavePage();
	}

	void RemoveInsuredPage::setInsured(std::vector<partial::insured_t> const& people) {
		QList<QStandardItem*> strings{};
		strings.reserve(static_cast<qsizetype>(people.size()));
		for (auto const& person : people) {
			auto name = name_from(person.first_name, person.last_name, true);
			std::string label{};
			if (person.document) {
				auto const kind = document_kind(person.kind && !person.kind->empty() ? person.kind->front() : '\0');
				label = std::format("{} ({}: {})", name, kind, *person.document);
			} else {
				label = std::move(name);
			}
			auto item = new QStandardItem{};
			item->setText(QString::fromUtf8(label));
			item->setCheckable(true);
			item->setEditable(false);
			item->setCheckState(Qt::Unchecked);
			strings.append(item);
		}

		auto model = new QStandardItemModel{this};
		model->appendColumn(strings);
		model->setHeaderData(0, Qt::Horizontal, "Ubezpieczeni");
		insuredList->setModel(model);

		QObject::connect(model, &QStandardItemModel::itemChanged, this, &RemoveInsuredPage::modelItemChanged);
	}

	void RemoveInsuredPage::configurationChanged() {
		auto const& formData = globals().data();
		if (formData.cfg.insured) {
			setInsured(formData.cfg.insured.value());
		} else {
			setInsured({});
		}
	}

	void RemoveInsuredPage::selectAllStateChanged(Qt::CheckState state) {
		if (state != Qt::PartiallyChecked) {
			selectAll->setTristate(false);
		}
		if (swappingOne) return;

		auto const model = qobject_cast<QStandardItemModel*>(insuredList->model());
		if (!model) return;

		selectingAll = true;

		auto const rows = model->rowCount();
		for (int rowIndex = 0; rowIndex < rows; ++rowIndex) {
			auto const item = model->item(rowIndex);
			if (!item) continue;
			item->setCheckState(state);
		}

		selectingAll = false;
	}

	void RemoveInsuredPage::swapCheckState(const QModelIndex& index) {
		auto const model = qobject_cast<QStandardItemModel*>(insuredList->model());
		if (!model) return;
		auto const item = model->item(index.row(), index.column());
		if (!item) return;
		item->setCheckState(item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);
	}

	void RemoveInsuredPage::modelItemChanged() {
		auto const model = qobject_cast<QStandardItemModel*>(insuredList->model());
		if (!model) return;

		int selectedRows = 0;
		auto const rows = model->rowCount();
		for (int rowIndex = 0; rowIndex < rows; ++rowIndex) {
			auto const item = model->item(rowIndex);
			if (!item) continue;
			if (item->checkState() == Qt::Checked) ++selectedRows;
		}

		setFormDirty(selectedRows != 0);
		if (!selectingAll) {
			swappingOne = true;
			selectAll->setCheckState(selectedRows == rows ? Qt::Checked
			                         : selectedRows == 0  ? Qt::Unchecked
			                                              : Qt::PartiallyChecked);
			swappingOne = false;
		}
	}

}  // namespace quick_dra::gui
