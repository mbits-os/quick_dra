// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QComboBox>
#include <QFormLayout>
#include <QObject>
#include <QWidget>
#include <span>
#include <string>
#include <string_view>
#include <utility>

namespace quick_dra::gui {
	class ComboBoxBase : public QObject {
		Q_OBJECT

	public:
		ComboBoxBase();

		std::string getCurrentDataText() const;
		void selectCurrentData(std::string_view text);

		void addToLayout(QWidget* parent,
		                 QFormLayout* layout,
		                 std::string_view label,
		                 std::span<std::pair<std::string_view, std::string_view> const> const& items);

		bool isValid() const noexcept { return true; }

	public slots:
		virtual void selectionChanged();

	signals:
		void valueChanged();
		void validationChanged();

	public:
		QFormLayout* parent{};
		QComboBox* combo{};
		bool attaching : 1 = false;
		bool dirty : 1 = false;
	};
}  // namespace quick_dra::gui
