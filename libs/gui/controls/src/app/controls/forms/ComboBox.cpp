// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <app/controls/forms/ComboBox.hpp>
#include <format>
#include <quick_dra/base/str.hpp>
#include <string>

namespace quick_dra::gui {
	ComboBoxBase::ComboBoxBase() = default;

	std::string ComboBoxBase::getCurrentDataText() const {
		return as_str(strip_sv(combo->currentData().toString().toUtf8()));
	}

	void ComboBoxBase::selectCurrentData(std::string_view text) {
		auto const index = combo->findData(QString::fromUtf8(text));
		combo->setCurrentIndex(index);
	}

	void ComboBoxBase::selectionChanged() {}

	void ComboBoxBase::addToLayout(QWidget* parentWidget,
	                               QFormLayout* layout,
	                               std::string_view label,
	                               std::span<std::pair<std::string_view, std::string_view> const> const& items) {
		parent = layout;
		combo = new QComboBox{parentWidget};
		for (auto const& [key, name] : items) {
			auto const itemLabel = std::format("{} - {}", key, name);
			combo->addItem(QString::fromUtf8(itemLabel), QString::fromUtf8(key));
		}
		combo->setCurrentIndex(-1);
		layout->addRow(QString::fromUtf8(label), combo);

		QObject::connect(combo, &QComboBox::currentIndexChanged, this, &ComboBoxBase::selectionChanged);
	}
}  // namespace quick_dra::gui
