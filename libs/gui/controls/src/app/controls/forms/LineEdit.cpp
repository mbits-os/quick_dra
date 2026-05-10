// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QApplication>
#include <app/controls/Glyph.hpp>
#include <app/controls/forms/LineEdit.hpp>
#include <app/gui/CurrentColor.hpp>
#include <app/utils/utils.hpp>

namespace quick_dra::gui {
	LineEditBase::LineEditBase() {}

	bool LineEditBase::isLightMode() {
		auto const& pal = qApp->palette();
		return pal.color(QPalette::Window).toHsl().lightness() > pal.color(QPalette::WindowText).toHsl().lightness();
	}

	std::string LineEditBase::getEditText() const { return as_str(strip_sv(edit->text().toUtf8())); }

	void LineEditBase::setValue(std::string_view text) { edit->setText(QString::fromUtf8(text)); }

	void LineEditBase::setValue(insurance_title const& title) {
		setValue(fmt::format("{} {} {}", title.title_code, title.pension_right, title.disability_level));
	}

	void LineEditBase::textChanged() {}

	void LineEditBase::addToLayout(QWidget* parentWidget, QFormLayout* layout, std::string_view label) {
		parent = layout;
		edit = new QLineEdit{parentWidget};

		QObject::connect(edit, &QLineEdit::textChanged, [self = this]() { self->textChanged(); });

		layout->addRow(QString::fromUtf8(label), edit);

		auto errorGlyph = new Glyph{parentWidget};
		errorGlyph->setIcon(warningSVGIcon());
		errorGlyph->setSizePolicy(FixedSize);
		errorLabel = new QLabel{parentWidget};
		errorLabel->setSizePolicy(TakeWidth / (HeightForWidth / 1_XStretch));
		errorLabel->setWordWrap(true);

		error = new QHBoxLayout{};
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
			default:
				break;
		};
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
