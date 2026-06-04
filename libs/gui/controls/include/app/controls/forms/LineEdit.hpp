// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QObject>
#include <QWidget>
#include <app/controls/forms/base.hpp>
#include <quick_dra/models/types.hpp>
#include <string>
#include <utility>

namespace quick_dra::gui {
	class LineEditBase : public QObject {
		Q_OBJECT

	public:
		LineEditBase();

		using Self = LineEditBase;

		static bool isLightMode();

		std::string getEditText() const;
		void setValue(std::string_view text);
		void setValue(insurance_title const& title);

		virtual void textChanged();

		void addToLayout(QWidget* parent, QFormLayout* layout, std::string_view label);
		void setValidation(Validation value, std::string_view error_message);

		bool isValid() const noexcept { return validation == Validation::Ok; }

		void restyleField(bool lightMode);

	signals:
		void valueChanged();
		void validationChanged();

	protected:
		void readValue(std::string& target, std::string&& editText) { target = std::move(editText); }
		void readValue(insurance_title& target, std::string&& editText) {
			insurance_title intermediate{};
			if (insurance_title::parse(editText, intermediate)) {
				target = std::move(intermediate);
			}
		}

	public:
		QFormLayout* parent{};
		QLineEdit* edit{};
		QHBoxLayout* error{};
		QLabel* errorLabel{};
		Validation validation{Validation::Ok};
		bool attaching : 1 = false;
		bool dirty : 1 = false;
	};

	template <Declaration DeclType>
	class LineEdit : public LineEditBase {
	public:
		LineEdit() = default;

		using LineEditBase::addToLayout;
		void addToLayout(QWidget* parentWidget, QFormLayout* layout) {
			addToLayout(parentWidget, layout, DeclType::label);
		}

		template <typename T>
		void connectTo(T* host) {
			QObject::connect(this, &LineEditBase::valueChanged, host, &T::updateCurrentValue);
			QObject::connect(this, &LineEditBase::validationChanged, host, &T::updateCurrentIsValid);
		}

		void textChanged() override {
			if (this->attaching) {
				return;
			}
			auto const text = this->getEditText();
			auto const result = DeclType::validate(text);
			this->dirty = true;
			this->setValidation(result, DeclType::error_message);
			this->valueChanged();
		}

		void attach(typename DeclType::target_type& target) {
			auto& text = DeclType::getField(target);
			attaching = true;
			setValue(text);
			attaching = false;
		}

		using LineEditBase::readValue;
		void readValue(typename DeclType::target_type& target) {
			if (!dirty) return;
			dirty = false;
			readValue(DeclType::getField(target), getEditText());
		}
	};
}  // namespace quick_dra::gui
