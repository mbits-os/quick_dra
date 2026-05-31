// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <app/controls/forms/DocumentComboBoxBase.hpp>
#include <format>

namespace quick_dra::gui {
	DocumentComboBoxBase::DocumentComboBoxBase() = default;

	void DocumentComboBoxBase::addToLayout(
	    QWidget* parent,
	    QFormLayout* layout,
	    std::span<std::pair<std::string_view, std::string_view> const> const& items) {
		kind.addToLayout(parent, layout, DocumentKindDeclaration::label, items);
		number.addToLayout(parent, layout, DocumentDeclaration::label);
	}

	void DocumentComboBoxBase::selectionChanged(std::string_view) {}

	void DocumentComboBoxBase::ComboBox::selectionChanged() {
		target->selectionChanged(getCurrentDataText());
		this->dirty = true;
		this->valueChanged();
	}

	DocumentComboBoxBase::LineEdit::LineEdit() = default;

	void DocumentComboBoxBase::LineEdit::textChanged() {
		if (this->attaching) {
			return;
		}
		auto const result = validator.validate(this->getEditText());
		this->dirty = true;
		if (result != Validation::Ok) {
			this->setValidation(result, validator.error_message);
		} else {
			std::string errorMessage{};
			if (blockChecker) {
				auto const nameFound = this->blockChecker(this->getEditText());
				if (nameFound) {
					errorMessage = std::format("{}: znaleziono inną ubezpieczoną osobę z tym dokumentem", *nameFound);
				}
			}
			if (errorMessage.empty()) {
				this->setValidation(Validation::Ok, {});
			} else {
				this->setValidation(Validation::Invalid, errorMessage);
			}
		}
		this->valueChanged();
	}

	void DocumentComboBoxBase::LineEdit::attach(typename DeclType::target_type& target) {
		auto& text = DeclType::getField(target);
		attaching = true;
		setValue(text);
		attaching = false;
	}

	void DocumentComboBoxBase::LineEdit::readValue(typename DeclType::target_type& target) {
		if (!dirty) return;
		dirty = false;
		auto text = getEditText();
		DeclType::getField(target) = std::move(text);
	}
}  // namespace quick_dra::gui
