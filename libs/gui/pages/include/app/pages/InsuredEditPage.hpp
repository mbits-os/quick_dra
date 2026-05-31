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
		void updateCurrentIsValid();
		void removeEmploymentHistoryEntries();
		void addNewEmploymentHistoryEntry();

	private:
		void storeNewHistory(std::vector<insured_type::history_type>&&);
		void updateValue(QString const& value, std::string& target);
		void restyleFields();

		template <typename Suite>
		Validation validate(std::string_view value) {
			auto const length = Suite::length();
			if (length != value.size()) {
				return length < value.size() ? Validation::TooShort : Validation::TooLong;
			}
			return Suite::is_valid(value) ? Validation::Ok : Validation::Invalid;
		}

		struct UI {
			QWidget* pageParent{};
			QFormLayout* formLayout{};
			LineEdit<FirstNameDeclaration> firstName{};
			LineEdit<LastNameDeclaration> lastName{};
			LineEdit<TitleDeclaration> title{};
			DocumentComboBox<IdCardEnumDeclaration, PassportEnumDeclaration, SocialIdEnumDeclaration> document{};
			ListView<EmploymentHistoryDeclaration,
			         EmploymentHistorySinceDeclaration,
			         EmploymentHistoryPartTimeScaleDeclaration,
			         EmploymentHistorySalaryDeclaration>
			    history{};

			void setupPageUI(InsuredEditPage* page);

			template <typename Cb>
			void each(Cb&& cb) {
				UI::each_impl(std::forward<Cb>(cb), firstName, lastName, title, document, history);
			}

			template <typename Cb>
			bool logical_and(Cb&& cb) {
				return UI::and_impl(std::forward<Cb>(cb), firstName, lastName, title, document, history);
			}

		private:
			template <typename... Items, typename Cb>
			static void each_impl(Cb&& cb, Items&&... items) {
				(cb(std::forward<Items>(items)), ...);
			}
			template <typename... Items, typename Cb>
			static bool and_impl(Cb&& cb, Items&&... items) {
				return (cb(std::forward<Items>(items)) && ...);
			}
		};

		UI ui{};
		size_t insuredIndex{};
		insured_type acceptedValue{};
		insured_type currentValue{};
	};
}  // namespace quick_dra::gui
