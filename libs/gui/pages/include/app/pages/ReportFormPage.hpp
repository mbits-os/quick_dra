// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QGridLayout>
#include <QScrollArea>
#include <app/controls/PageScrollArea.hpp>
#include <app/gui/PagedWidget.hpp>
#include <app/utils/FormData.hpp>

namespace quick_dra::gui {
	class ReportFormPage : public PagedWidget {
		Q_OBJECT

	public:
		explicit ReportFormPage(size_t index, QWidget* parent = nullptr);
		~ReportFormPage();

		void connectPage() override;
		bool event(QEvent*) override;

	private:
		struct UI {
			QWidget* gridParent{};
			QGridLayout* gridLayout{};

			void setupUI(QWidget*);
		};
		UI ui{};
		size_t index{};
	};
}  // namespace quick_dra::gui
