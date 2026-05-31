// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QCheckBox>
#include <QTreeView>
#include <app/gui/PagedWidget.hpp>
#include <app/utils/forms.hpp>
#include <functional>
#include <vector>

#undef emit
#include <quick_dra/models/types.hpp>

namespace quick_dra::gui {
	class RemoveHistoryPage : public PagedWidget {
		Q_OBJECT

	public:
		explicit RemoveHistoryPage(
		    std::vector<insured_type::history_type> const& history,
		    std::function<void(std::vector<insured_type::history_type>&&)> const& acceptNewHistory,
		    QWidget* parent = nullptr);
		~RemoveHistoryPage();

		void accept() override;

	private slots:
		void selectAllStateChanged(Qt::CheckState);
		void swapCheckState(const QModelIndex& index);
		void modelItemChanged();

	private:
		void setupUI();
		void buildModel();

		std::vector<insured_type::history_type> history;
		std::function<void(std::vector<insured_type::history_type>&&)> acceptNewHistory;
		QCheckBox* selectAll{};
		QTreeView* historyList{};
		bool selectingAll : 1 = false;
		bool swappingOne : 1 = false;
	};
}  // namespace quick_dra::gui
