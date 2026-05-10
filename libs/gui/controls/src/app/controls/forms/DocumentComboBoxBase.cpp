// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <app/controls/forms/DocumentComboBoxBase.hpp>

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
}  // namespace quick_dra::gui
