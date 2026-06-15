// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <app/controls/Forms.hpp>
#include <app/controls/Glyph.hpp>
#include <app/gui/CurrentColor.hpp>
#include <app/gui/PagedWidget.hpp>
#include <app/utils/utils.hpp>
#include <quick_dra/lex/validators.hpp>
#include <quick_dra/models/types.hpp>
#include <string>
#include <utility>

namespace quick_dra::gui {
	class InsuredEditPage : public PagedWidget {
		Q_OBJECT

	public:
		explicit InsuredEditPage(size_t index, QWidget* parent = nullptr);
		~InsuredEditPage();

		bool event(QEvent*) override;
		void connectPage() override;
		void accept() override;

	public slots:
		void updateCurrentValue();
		void updateFormValid();
		void removeEmploymentHistoryEntries();
		void addNewEmploymentHistoryEntry();

	private:
		void storeNewHistory(std::vector<insured_type::history_type>&&);

		template <typename Suite>
		Validation validate(std::string_view value) {
			auto const length = Suite::length();
			if (length != value.size()) {
				return length < value.size() ? Validation::TooShort : Validation::TooLong;
			}
			return Suite::is_valid(value) ? Validation::Ok : Validation::Invalid;
		}

		using HistoryListView = ListView<EmploymentHistoryDeclaration,
		                                 EmploymentHistorySinceDeclaration,
		                                 EmploymentHistoryPartTimeScaleDeclaration,
		                                 EmploymentHistorySalaryDeclaration>;
		using DocumentCombo = DocumentComboBox<IdCardEnumDeclaration, PassportEnumDeclaration, SocialIdEnumDeclaration>;
		using Form = FormUI<LineEdit<FirstNameDeclaration>,
		                    LineEdit<LastNameDeclaration>,
		                    LineEdit<TitleDeclaration>,
		                    DocumentCombo,
		                    HistoryListView>;
		struct UI : Form {
			void setupPageUI(InsuredEditPage* page);
			HistoryListView& history() { return get<HistoryListView>(); }
			DocumentCombo& document() { return get<DocumentCombo>(); }
		};

		UI ui{};
		size_t insuredIndex{};
		insured_type acceptedValue{};
		insured_type currentValue{};
	};
}  // namespace quick_dra::gui
