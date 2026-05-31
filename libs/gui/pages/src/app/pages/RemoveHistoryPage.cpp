// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QDate>
#include <QStandardItemModel>
#include <app/controls/PageScrollArea.hpp>
#include <app/controls/PanelButtonGroup.hpp>
#include <app/gui/CurrentColor.hpp>
#include <app/gui/PageStack.hpp>
#include <app/pages/InsuredEditPage.hpp>
#include <app/pages/PayerEditPage.hpp>
#include <app/pages/RemoveHistoryPage.hpp>
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
#include <string_view>
#include <utility>

using namespace std::literals;

namespace quick_dra::gui {
	RemoveHistoryPage::RemoveHistoryPage(
	    std::vector<insured_type::history_type> const& history,
	    std::function<void(std::vector<insured_type::history_type>&&)> const& acceptNewHistory,
	    QWidget* parent)
	    : PagedWidget(parent), history{history}, acceptNewHistory{acceptNewHistory} {
		setupUI();
	}

	RemoveHistoryPage::~RemoveHistoryPage() = default;

	void RemoveHistoryPage::setupUI() {
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
		    .createWidget(historyList, "historyList", [](auto& wgt) {
			    wgt.setSizePolicy(TakeWidth);
			    wgt.setIndentation(0);
		    });

		QObject::connect(selectAll, &QCheckBox::checkStateChanged, this, &RemoveHistoryPage::selectAllStateChanged);
		QObject::connect(historyList, &QTreeView::activated, this, &RemoveHistoryPage::swapCheckState);

		buildModel();
	}

	QVariant displayRole(ratio scale) {
		if (scale.num == scale.den) return "Pełny";
		return QString::fromUtf8(ratio_from(scale.num, scale.den));
	}

	QVariant displayRole(currency salary) {
		if (salary == minimal_salary) return "Minimalna";
		auto const value = static_cast<double>(salary.value) / 100.0;
		return QString::fromUtf8("%1 zł"sv).arg(QLocale::system().toString(value, 'f', 2));
	}

	void RemoveHistoryPage::buildModel() {
		QList<QStandardItem*> strings{};
		strings.reserve(static_cast<qsizetype>(history.size()));
		for (auto const& entry : history) {
			auto const [num, den] = entry.part_time_scale;
			auto const value = static_cast<double>(entry.salary.value) / 100.0;
			auto const salary = num == den
			                        ? entry.salary == minimal_salary
			                              ? "Minimalna krajowa"
			                              : QString::fromUtf8("%1 zł"sv).arg(QLocale::system().toString(value, 'f', 2))
			                    : entry.salary == minimal_salary
			                        ? QString{"%1 minimalnej krajowej"}.arg(QString::fromUtf8(ratio_from(num, den)))
			                        : QString::fromUtf8("%1 z %2 zł"sv)
			                              .arg(QString::fromUtf8(ratio_from(num, den)))
			                              .arg(QLocale::system().toString(value, 'f', 2));
			auto const since =
			    entry.since.year() < 1900y
			        ? QString{}
			        : QString{" (od %1)"}.arg(QLocale::system().toString(QDate{entry.since / 1d}, "MMM yyyy"));

			auto item = new QStandardItem{};
			item->setText(salary + since);
			item->setCheckable(true);
			item->setEditable(false);
			item->setCheckState(Qt::Unchecked);
			strings.append(item);
		}

		auto model = new QStandardItemModel{this};
		model->appendColumn(strings);
		model->setHeaderData(0, Qt::Horizontal, "Historia");
		historyList->setModel(model);

		QObject::connect(model, &QStandardItemModel::itemChanged, this, &RemoveHistoryPage::modelItemChanged);
	}

	void RemoveHistoryPage::accept() {
		auto const model = qobject_cast<QStandardItemModel*>(historyList->model());
		if (model) {
			std::vector<insured_type::history_type> saved{};
			auto const rows = model->rowCount();
			saved.reserve(static_cast<size_t>(rows));
			for (int rowIndex = 0; rowIndex < rows; ++rowIndex) {
				auto const item = model->item(rowIndex);
				if (!item) continue;
				if (item->checkState() == Qt::Unchecked) {
					saved.push_back(std::move(history[static_cast<size_t>(rowIndex)]));
				}
			}

			acceptNewHistory(std::move(saved));
		}

		selectAll->setChecked(false);
		setFormDirty(false);
		leavePage();
	}

	void RemoveHistoryPage::selectAllStateChanged(Qt::CheckState state) {
		if (state != Qt::PartiallyChecked) {
			selectAll->setTristate(false);
		}
		if (swappingOne) return;

		auto const model = qobject_cast<QStandardItemModel*>(historyList->model());
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

	void RemoveHistoryPage::swapCheckState(const QModelIndex& index) {
		auto const model = qobject_cast<QStandardItemModel*>(historyList->model());
		if (!model) return;
		auto const item = model->item(index.row(), index.column());
		if (!item) return;
		item->setCheckState(item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);
	}

	void RemoveHistoryPage::modelItemChanged() {
		auto const model = qobject_cast<QStandardItemModel*>(historyList->model());
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
