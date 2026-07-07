// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QDate>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLocale>
#include <algorithm>
#include <app/controls/forms/TableView.hpp>
#include <app/utils/str.hpp>
#include <app/utils/utils.hpp>
#include <format>
#include <quick_dra/docs/locale.hpp>
#include <quick_dra/models/project_reader.hpp>
#include <string>

namespace quick_dra::gui {
	namespace {
		template <size_t... Index>
		std::string_view strip_suffix(std::string_view input,
		                              std::string_view suffix,
		                              std::same_as<std::string_view> auto... suffixes) {
			if (input.ends_with(suffix)) {
				return strip_sv(input.substr(0, input.size() - suffix.size()));
			}
			if constexpr (sizeof...(suffixes) > 0) {
				return strip_suffix(input, suffixes...);
			} else {
				return input;
			}
		}
	}  // namespace

	namespace detail {
		QVariant displayRole(year_month month) {
			if (month.year() < 1900y) return "Od zawsze";
			return QLocale{}.toString(QDate{month / 1d}, "MMM yyyy");
		}

		QVariant displayRole(ratio scale) {
			if (scale.num == scale.den) return "Pełny";
			return QString::fromUtf8(ratio_from(scale.num, scale.den));
		}

		QVariant displayRole(currency salary) {
			if (salary == minimal_salary) return "Minimalna";
			return QString::fromUtf8(locale::from_system(salary));
		}

		QVariant editRole(year_month month) {
			return QString::fromUtf8(
			    std::format("{:04}/{:02}", static_cast<int>(month.year()), static_cast<unsigned>(month.month())));
		}

		QVariant editRole(ratio scale) {
			auto [num, den] = scale;
			auto const value = std::gcd(num, den);
			num /= value;
			den /= value;
			return QString::fromUtf8(std::format("{}/{}", num, den));
		}

		QVariant editRole(currency salary) {
			return QString::fromUtf8(locale::number_grouping::monetary_from_locale().group(salary));
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

			text = strip_suffix(text, "zł"sv, "pln"sv, "ZŁ"sv, "PLN"sv);
			auto ok = false;
			auto const doubleValue = QLocale{}.toDouble(QString::fromUtf8(text), &ok);
			if (ok) {
				auto const rawValue = static_cast<long long>(doubleValue * calc_currency::den);
				salary = calc_currency{rawValue}.rounded();
				return true;
			}
			return false;
		}
	}  // namespace detail

	ListViewBase::ListViewBase() = default;

	void ListViewBase::addToLayout(QWidget* parentWidget,
	                               QFormLayout* layout,
	                               std::string_view label,
	                               std::string_view id,
	                               QAbstractItemModel* model) {
		auto const qId = QString::fromUtf8(id);

		parent = layout;

		auto buttonLayout = new QHBoxLayout{};
		buttonLayout->setObjectName(QString{"%1ButtonLayout"}.arg(qId));
		buttonLayout->setContentsMargins(0, 0, 0, 0);
		buttonLayout->setAlignment(Qt::AlignRight);

		static constexpr auto buttonStyle = R"(
QPushButton
{
  padding: 10px 20px;
}
)";
		addButton = new QPushButton{parentWidget};
		addButton->setObjectName(QString{"%1AddButton"}.arg(qId));
		addButton->setText("Dodaj wpis");
		addButton->setSizePolicy(FixedSize);
		addButton->setStyleSheet(buttonStyle);
		buttonLayout->addWidget(addButton);

		removeButton = new QPushButton{parentWidget};
		removeButton->setObjectName(QString{"%1RemoveButton"}.arg(qId));
		removeButton->setText("Usuń wpis");
		removeButton->setSizePolicy(FixedSize);
		removeButton->setStyleSheet(buttonStyle);
		buttonLayout->addWidget(removeButton);

		layout->addRow(QString::fromUtf8(label), buttonLayout);
		auto const labelWidget = layout->itemAt(layout->rowCount() - 1, QFormLayout::LabelRole)->widget();

		view = new QTreeView{parentWidget};
		view->setObjectName(QString{"%1TreeView"}.arg(qId));
		view->setSizePolicy(TakeAll);
		view->setModel(model);
		view->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
		view->setRootIsDecorated(true);
		view->setSortingEnabled(true);
		view->setIndentation(0);
		view->sortByColumn(0, Qt::AscendingOrder);
		view->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

		layout->addRow("", view);

		static_cast<QLabel*>(labelWidget)->setBuddy(view);
	}

	void ListViewBase::restyleField(bool) {}  // GCOV_EXCL_LINE -- no side effects to test for

}  // namespace quick_dra::gui
