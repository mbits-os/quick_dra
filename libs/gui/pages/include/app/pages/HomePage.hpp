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
	class HomePage : public PagedWidget {
		Q_OBJECT

	public:
		explicit HomePage(QWidget* parent = nullptr);
		~HomePage();

		void connectPage() override;
		bool survivesReload() const override { return true; }
		PanelButtonGroup* reportButtons() const { return summaryGroup; }

	public slots:
		void editReportIdAction();
		void showPersonelFilesAction();
		void storeKeduXmlLocally();

		void reportIdAccepted(int serial, QDate const& date, bool moved);
		void reportIdChanged();
		void formSetChanged();

	private:
		void setupUI();

		void updateSummaryIdentifier();
		PanelButton* layoutFormReference(PanelButtonGroup* group,
		                                 FormData::FormRef const&,
		                                 std::function<void()> const& slot);
		void pushFormView(size_t index);

		PanelButtonGroup* summaryGroup{};
		QLabel* summaryIdentifier{};
	};
}  // namespace quick_dra::gui
