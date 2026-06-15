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
	class PayerEditPage : public PagedWidget {
		Q_OBJECT

	public:
		explicit PayerEditPage(QWidget* parent = nullptr);
		~PayerEditPage();

		bool event(QEvent*) override;
		void connectPage() override;
		void accept() override;

	public slots:
		void updateCurrentValue();
		void updateFormValid();

	private:
		template <typename Suite>
		Validation validate(std::string_view value) {
			auto const length = Suite::length();
			if (length != value.size()) {
				return length < value.size() ? Validation::TooShort : Validation::TooLong;
			}
			return Suite::is_valid(value) ? Validation::Ok : Validation::Invalid;
		}

		using UI = FormUI<LineEdit<FirstNameDeclaration>,
		                  LineEdit<LastNameDeclaration>,
		                  LineEdit<TaxIdDeclaration>,
		                  LineEdit<SocialIdDeclaration>,
		                  DocumentComboBox<IdCardEnumDeclaration, PassportEnumDeclaration>>;

		UI ui{};
		payer_t acceptedValue{};
		payer_t currentValue{};
	};
}  // namespace quick_dra::gui
