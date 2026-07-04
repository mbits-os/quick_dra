// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QApplication>
#include <app/controls/Glyph.hpp>
#include <app/controls/forms/LineEdit.hpp>
#include <app/gui/CurrentColor.hpp>
#include <app/utils/str.hpp>
#include <app/utils/utils.hpp>
#include <format>
#include <string>

namespace quick_dra::gui {
	LineEditBase::LineEditBase() {}

	bool LineEditBase::isLightMode() {
		auto const& pal = qApp->palette();
		return pal.color(QPalette::Window).toHsl().lightness() > pal.color(QPalette::WindowText).toHsl().lightness();
	}

	std::string LineEditBase::getEditText() const { return as_str(strip_sv(edit->text().toUtf8())); }

	void LineEditBase::setValue(std::string_view text) { edit->setText(QString::fromUtf8(text)); }

	void LineEditBase::setValue(insurance_title const& title) {
		setValue(std::format("{} {} {}", title.title_code, title.pension_right, title.disability_level));
	}

	void LineEditBase::textChanged() {}  // GCOV_EXCL_LINE

	void LineEditBase::addToLayout(QWidget* parentWidget,
	                               QFormLayout* layout,
	                               std::string_view label,
	                               std::string_view id) {
		auto const qId = QString::fromUtf8(id);

		parent = layout;
		edit = new QLineEdit{parentWidget};
		edit->setObjectName(QString{"%1Edit"}.arg(qId));
		edit->setTextMargins(2, 4, 2, 4);

		QObject::connect(edit, &QLineEdit::textChanged, [self = this]() { self->textChanged(); });

		layout->addRow(QString::fromUtf8(label), edit);

		auto errorGlyph = new Glyph{parentWidget};
		errorGlyph->setObjectName(QString{"%1ErrorGlyph"}.arg(qId));
		errorGlyph->setIcon(warningSVGIcon());
		errorGlyph->setSizePolicy(FixedSize);
		errorLabel = new QLabel{parentWidget};
		errorLabel->setObjectName(QString{"%1ErrorLabel"}.arg(qId));
		errorLabel->setSizePolicy(TakeWidth / (HeightForWidth / 1_XStretch));
		errorLabel->setWordWrap(true);

		error = new QHBoxLayout{};
		error->setObjectName(QString{"%1Error"}.arg(qId));
		error->setContentsMargins(0, 0, 0, 0);
		error->addWidget(errorGlyph);
		error->addWidget(errorLabel);
		layout->addRow("", error);
		layout->setRowVisible(error, false);
	}

	void LineEditBase::setValidation(Validation value, std::string_view error_message) {
		if (validation == value) return;

		validation = value;

		std::string_view message{};
		switch (validation) {
			case Validation::Empty:
				message = "Pole nie może być puste"sv;
				break;
			case Validation::TooShort:
				[[fallthrough]];
			case Validation::TooLong:
				[[fallthrough]];
			case Validation::Invalid:
				message = error_message.empty() ? "Pole jest niepoprawne"sv : error_message;
				break;
			default:    // GCOV_EXCL_LINE
				break;  // GCOV_EXCL_LINE
		}
		edit->setToolTip(QString::fromUtf8(message));
		errorLabel->setText(QString::fromUtf8(message));
		parent->setRowVisible(error, !message.empty());
		restyleField(isLightMode());

		validationChanged();
	}

	void LineEditBase::restyleField(bool lightMode) {
		edit->setStyleSheet(isValid() ? "" : lightMode ? "background-color: #ffcccc" : "background-color: #551111");
	}
}  // namespace quick_dra::gui
