// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QCheckBox>
#include <QTreeView>
#include <app/gui/PagedWidget.hpp>
#include <vector>

#undef emit
#include <quick_dra/models/types.hpp>

namespace quick_dra::gui {
	class RemoveInsuredPage : public PagedWidget {
		Q_OBJECT

	public:
		explicit RemoveInsuredPage(QWidget* parent = nullptr);
		~RemoveInsuredPage();

		void connectPage() override;
		void accept() override;
		bool survivesReload() const override { return true; }

	public slots:
		void configurationChanged();

	private slots:
		void selectAllStateChanged(Qt::CheckState);
		void swapCheckState(const QModelIndex& index);
		void modelItemChanged();

	private:
		void setupUI();

		void setInsured(std::vector<partial::insured_t> const&);

		QCheckBox* selectAll{};
		QTreeView* insuredList{};
		bool selectingAll : 1 = false;
		bool swappingOne : 1 = false;
	};
}  // namespace quick_dra::gui
