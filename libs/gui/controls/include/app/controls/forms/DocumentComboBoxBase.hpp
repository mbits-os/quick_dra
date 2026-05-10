// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <app/controls/forms/ComboBox.hpp>
#include <app/controls/forms/LineEdit.hpp>
#include <app/controls/forms/base.hpp>
#include <app/controls/forms/field.hpp>

namespace quick_dra::gui {
	class DocumentComboBoxBase : QObject {
		Q_OBJECT;

	public:
		DocumentComboBoxBase();

		void addToLayout(QWidget* parent,
		                 QFormLayout* layout,
		                 std::span<std::pair<std::string_view, std::string_view> const> const& items);

	public:
		DECLARE_FIELD(DocumentKindDeclaration, detail::NullValidator, person, kind) {
			static constexpr auto label = "Rodzaj dokumentu"sv;
			static constexpr auto error_message = ""sv;
		};
		DECLARE_FIELD(DocumentDeclaration, detail::NullValidator, person, document) {
			static constexpr auto label = "Seria i numer dokumentu"sv;
			static constexpr auto error_message = ""sv;
		};

		struct DocumentValidator {
			Validation (*validate)(std::string_view) = [](std::string_view) { return Validation::Ok; };
			std::string_view error_message = ""sv;
		};

		class LineEdit : public LineEditBase {
		public:
			using DeclType = DocumentDeclaration;

			LineEdit() = default;

			using LineEditBase::addToLayout;
			void addToLayout(QWidget* parentWidget, QFormLayout* layout) {
				addToLayout(parentWidget, layout, DeclType::label);
			}

			template <typename T>
			void swapValidator() {
				validator = {
				    .validate = T::validate,
				    .error_message = T::error_message,
				};
			}

			void setValidator(DocumentValidator const& next) { validator = next; }

			template <typename T>
			void connectTo(T* host) {
				QObject::connect(this, &LineEditBase::valueChanged, host, &T::updateCurrentValue);
				QObject::connect(this, &LineEditBase::validationChanged, host, &T::updateCurrentIsValid);
			}

			void textChanged() override {
				if (this->attaching) {
					return;
				}
				auto const result = validator.validate(this->getEditText());
				this->dirty = true;
				this->setValidation(result, validator.error_message);
				this->valueChanged();
			}

			void attach(typename DeclType::target_type& target) {
				auto& text = DeclType::getField(target);
				attaching = true;
				setValue(text);
				attaching = false;
			}

			void readValue(typename DeclType::target_type& target) {
				if (!dirty) return;
				dirty = false;
				auto text = getEditText();
				DeclType::getField(target) = std::move(text);
			}

			DocumentValidator validator{};
		};

		class ComboBox : public ComboBoxBase {
		public:
			ComboBox(DocumentComboBoxBase* target) : target{target} {}
			void selectionChanged() override;
			DocumentComboBoxBase* target;

			void attach(person& tgt) {
				attaching = true;
				selectCurrentData(tgt.kind);
				attaching = false;
			}

			void readValue(person& tgt) {
				if (!dirty) return;
				dirty = false;
				tgt.kind = std::move(getCurrentDataText());
			}

			template <typename T>
			void connectTo(T* host) {
				QObject::connect(this, &ComboBoxBase::valueChanged, host, &T::updateCurrentValue);
				QObject::connect(this, &ComboBoxBase::validationChanged, host, &T::updateCurrentIsValid);
			}
		};

		virtual void selectionChanged(std::string_view key);

		bool isValid() const noexcept { return kind.isValid() && number.isValid(); }
		void restyleField(bool lightMode) { number.restyleField(lightMode); }

		ComboBox kind{this};
		LineEdit number{};
	};

	struct KindStorage {
		std::string currentValue{};
		Validation validation{Validation::Ok};
		DocumentComboBoxBase::DocumentValidator validator{};

		template <Declaration DeclType>
		static KindStorage from() {
			return {.validator = {
			            .validate = DeclType::validate,
			            .error_message = DeclType::error_message,
			        }};
		}
	};

	template <typename T>
	using kind_for = KindStorage;

	template <typename... T>
	using storage_for = std::tuple<KindStorage, kind_for<T>...>;

	template <DocumentKindDeclaration... DeclTypes>
	class DocumentComboBox : public DocumentComboBoxBase {
	public:
		DocumentComboBox() = default;

		using Storage = storage_for<DeclTypes...>;

		using DocumentComboBoxBase::addToLayout;
		void addToLayout(QWidget* parent, QFormLayout* layout) {
			addToLayout(parent, layout, std::array{DocumentComboBox::comboBoxItem<DeclTypes>()...});
		}

		void attach(person& target) {
			currentKey = target.kind;
			DocumentComboBox::currentStorage().currentValue = target.document;
			number.attach(target);
			kind.attach(target);
		}

		void readValue(person& target) {
			number.readValue(target);
			kind.readValue(target);
		}

		void connectTo(auto* host) {
			number.connectTo(host);
			kind.connectTo(host);
		}

		void selectionChanged(std::string_view key) override {
			if (key == currentKey) {
				return;
			}
			auto& current = DocumentComboBox::currentStorage();
			current.currentValue = number.getEditText();
			current.validation = current.validator.validate(current.currentValue);

			currentKey = key;
			auto& next = DocumentComboBox::currentStorage();
			number.setValidator(next.validator);
			number.setValue(next.currentValue);
		}

		std::string currentKey{};
		Storage storage{KindStorage::from<DeclTypes>()..., {}};

	private:
		template <typename DeclType>
		static std::pair<std::string_view, std::string_view> comboBoxItem() noexcept {
			return {DeclType::enum_key, DeclType::label};
		}

		KindStorage& storageFor(std::string_view key) noexcept { return storageForImpl<0, DeclTypes...>(key); }
		KindStorage& currentStorage() noexcept { return storageFor(currentKey); }

		template <size_t N, typename Head, typename... Tail>
		KindStorage& storageForImpl(std::string_view key) noexcept {
			if (key == Head::enum_key) {
				return std::get<N>(this->storage);
			}
			if constexpr (sizeof...(Tail) > 0) {
				return this->storageForImpl<N + 1, Tail...>(key);
			} else {
				return std::get<N + 1>(this->storage);
			}
		};
	};
}  // namespace quick_dra::gui
