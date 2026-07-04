// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <app/controls/forms/ComboBox.hpp>
#include <app/utils/str.hpp>
#include <format>
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

	void ComboBoxBase::selectionChanged() {}  // GCOV_EXCL_LINE

	void ComboBoxBase::addToLayout(QWidget* parentWidget,
	                               QFormLayout* layout,
	                               std::string_view label,
	                               std::span<std::pair<std::string_view, std::string_view> const> const& items,
	                               std::string_view id) {
		parent = layout;
		combo = new QComboBox{parentWidget};
		combo->setObjectName(QString{"%1ComboBox"}.arg(QString::fromUtf8(id)));
		for (auto const& [key, name] : items) {
			auto const itemLabel = std::format("{} - {}", key, name);
			combo->addItem(QString::fromUtf8(itemLabel), QString::fromUtf8(key));
		}
		combo->setCurrentIndex(-1);
		combo->setStyleSheet(
		    R"(
QComboBox
{
  padding: 5px;
  padding-left: 8px;
}

QComboBox QAbstractItemView::item {
  padding-top: 5px;
  padding-bottom: 5px;
}
)");
		layout->addRow(QString::fromUtf8(label), combo);

		QObject::connect(combo, &QComboBox::currentIndexChanged, this, &ComboBoxBase::selectionChanged);
	}
}  // namespace quick_dra::gui
