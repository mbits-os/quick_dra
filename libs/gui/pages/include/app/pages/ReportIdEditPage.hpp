// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QDate>
#include <app/gui/PagedWidget.hpp>
#include <app/utils/FormData.hpp>
#include <quick_dra/base/chrono.hpp>

namespace quick_dra::gui::Ui {
	class ReportIdEditPage;
}

namespace quick_dra::gui {
	class ReportIdEditPage : public PagedWidget {
		Q_OBJECT

	public:
		explicit ReportIdEditPage(QWidget* parent = nullptr);
		~ReportIdEditPage();

		void accept() override;
		bool survivesReload() const override { return true; }

		void initialData(ReportId const& reportId);
		unsigned idIndex() const noexcept { return currentId.index; }
		bool isOverriden() const noexcept { return currentId.isOverriden; }
		year_month const& date() const noexcept { return currentId.date; }
		void setDate(year_month const& value);

	signals:
		void identifierUpdated(int serial, QDate const& date, bool moved);
		void serialChanged(int value);
		void dateChanged(QDate const& value);
		void movedChanged(bool value);

	public slots:
		void setSerial(int value);
		void setDate(QDate const& value);
		void resetMoved();
		void setMoved(bool value);

	private:
		Ui::ReportIdEditPage* ui;

		ReportId acceptedId{};
		ReportId currentId{};
	};
}  // namespace quick_dra::gui
