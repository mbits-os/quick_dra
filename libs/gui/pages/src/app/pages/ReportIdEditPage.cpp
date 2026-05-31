// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <algorithm>
#include <app/gui/CurrentColor.hpp>
#include <app/gui/PageStack.hpp>
#include <app/pages/ReportIdEditPage.hpp>
#include <app/utils/LaidOut.hpp>
#include <app/utils/utils.hpp>
#include <quick_dra/base/chrono.hpp>
#include "ui_ReportIdEditPage.h"

namespace quick_dra::gui {
	namespace Ui {
		class ReportIdEditPage : public Ui_ReportIdEditPage {};
	}  // namespace Ui

	ReportIdEditPage::ReportIdEditPage(QWidget* parent) : PagedWidget(parent), ui(new Ui::ReportIdEditPage) {
		ui->setupUi(this);

		QObject::connect(ui->spinBox, SIGNAL(valueChanged(int)), this, SLOT(setSerial(int)));
		QObject::connect(ui->dateEdit, SIGNAL(userDateChanged(QDate)), this, SLOT(setDate(QDate)));
		QObject::connect(this, SIGNAL(movedChanged(bool)), ui->toolButton, SLOT(setEnabled(bool)));
		QObject::connect(ui->toolButton, SIGNAL(clicked()), this, SLOT(resetMoved()));
		QObject::connect(this, SIGNAL(dateChanged(QDate)), ui->dateEdit, SLOT(setDate(QDate)));
		QObject::connect(this, SIGNAL(serialChanged(int)), ui->spinBox, SLOT(setValue(int)));

		ui->toolButton->setIcon(resetSVGIcon());
		restrictToolButton(ui->toolButton, 30);
	}

	ReportIdEditPage::~ReportIdEditPage() { delete ui; }

	void ReportIdEditPage::initialData(ReportId const& reportId) {
		currentId = reportId;
		serialChanged(static_cast<int>(currentId.index));
		dateChanged(currentId.date / 1d);
		movedChanged(currentId.isOverriden);
		acceptedId = currentId;
		setFormDirty(false);
	}

	void ReportIdEditPage::accept() {
		identifierUpdated(static_cast<int>(currentId.index), currentId.date / 1d, currentId.isOverriden);
		acceptedId = currentId;
		setFormDirty(false);
		leavePage();
	}

	void ReportIdEditPage::setSerial(int value) {
		auto const index = static_cast<unsigned>(std::min(std::max(value, 1), 99));

		if (currentId.index == index) {
			return;
		}
		currentId.index = index;

		setFormDirty(currentId != acceptedId);
		serialChanged(static_cast<int>(currentId.index));
	}

	void ReportIdEditPage::setDate(year_month const& value) {
		if (currentId.date == value) {
			return;
		}
		currentId.date = value;

		setFormDirty(currentId != acceptedId);
		dateChanged(currentId.date / 1d);
		setMoved(true);
	}

	void ReportIdEditPage::setDate(QDate const& value) {
		auto const ymd = year_month_day{value.toStdSysDays()};
		setDate(ymd.year() / ymd.month());
	}

	void ReportIdEditPage::resetMoved() {
		setDate(month_today() - months{1});
		setMoved(false);
	}

	void ReportIdEditPage::setMoved(bool value) {
		if (currentId.isOverriden == value) {
			return;
		}
		currentId.isOverriden = value;

		setFormDirty(currentId != acceptedId);
		movedChanged(currentId.isOverriden);
	}
}  // namespace quick_dra::gui
