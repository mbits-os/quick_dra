// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QDate>
#include <QFormLayout>
#include <QLocale>
#include <app/controls/forms/TableView.hpp>
#include <app/utils/utils.hpp>
#include <quick_dra/base/str.hpp>
#include <quick_dra/models/project_reader.hpp>

namespace quick_dra::gui {
	namespace detail {
		QVariant displayRole(year_month month) {
			if (month.year() < 1900y) return "Od zawsze";
			return QLocale::system().toString(QDate{month / 1d}, "MMM yyyy");
		}

		QVariant displayRole(ratio scale) {
			if (scale.num == scale.den) return "Pełny";
			return QString::fromUtf8(ratio_from(scale.num, scale.den));
		}

		QVariant displayRole(currency salary) {
			if (salary == minimal_salary) return "Minimalna";
			auto const value = static_cast<double>(salary.value) / 100.0;
			return QLocale::system().toCurrencyString(value, QString::fromUtf8("zł"), 2);
		}

		QVariant editRole(year_month month) {
			return QString::fromUtf8(
			    fmt::format("{:04}/{:02}", static_cast<int>(month.year()), static_cast<unsigned>(month.month())));
		}

		QVariant editRole(ratio scale) {
			auto [num, den] = scale;
			auto const value = std::gcd(num, den);
			num /= value;
			den /= value;
			return QString::fromUtf8(fmt::format("{}/{}", num, den));
		}

		QVariant editRole(currency salary) {
			auto const value = std::max(0.0, static_cast<double>(salary.value) / 100.0);
			return QLocale::system().toString(value, 'f', 2);
		}

		bool readEditValue(year_month& month, std::string_view text) {
			year_month local;
			if (yaml::convert_string(text, local)) {
				month = local;
				return true;
			}
			return false;
		}

		bool readEditValue(ratio& scale, std::string_view text) {
			ratio local;
			if (ratio::parse(text, local) && local.den != 0) {
				scale = local;
				return true;
			}

			return false;
		}

		bool readEditValue(currency& salary, std::string_view text) {
			std::string lower = to_lower(text);

			if (lower == "minimalna"sv || lower == "minimalny"sv || lower == "minimalne"sv || lower == "minimal"sv) {
				salary = minimal_salary;
				return true;
			}

			currency local{};
			if (currency::parse(text, local) && local >= 0_PLN) {
				salary = local;
				return true;
			}
			return false;
		}
	}  // namespace detail

	ListViewBase::ListViewBase() = default;

	void ListViewBase::addToLayout(QWidget* parentWidget,
	                               QFormLayout* layout,
	                               std::string_view label,
	                               QAbstractItemModel* model) {
		parent = layout;
		view = new QTreeView{parentWidget};
		view->setSizePolicy(TakeWidth);
		view->setModel(model);
		view->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
		view->setRootIsDecorated(true);
		view->setSortingEnabled(true);
		view->setIndentation(0);

		// QObject::connect(edit, &QLineEdit::textChanged, [self = this]() { self->textChanged(); });

		layout->addRow(QString::fromUtf8(label), view);
	}

	void ListViewBase::restyleField(bool) {}

}  // namespace quick_dra::gui
