// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QFormLayout>
#include <QWidget>
#include <app/controls/PageScrollArea.hpp>
#include <app/controls/forms/ComboBox.hpp>
#include <app/controls/forms/DocumentComboBoxBase.hpp>
#include <app/controls/forms/LineEdit.hpp>
#include <app/controls/forms/TableView.hpp>
#include <app/controls/forms/base.hpp>
#include <app/controls/forms/field.hpp>
#include <tuple>
#include <utility>

namespace quick_dra::gui {
	template <typename... Fields>
	struct FormUI {
		QWidget* pageParent{};
		QFormLayout* formLayout{};
		std::tuple<Fields...> fields{};

		template <typename PageType>
		void setupPageUI(PageType* page) {
			auto const [_, pageParentPtr] = PageScrollArea::setupPage(page);
			this->pageParent = pageParentPtr;
			this->formLayout = new QFormLayout(this->pageParent);
			each([self = this, page](auto& item) {
				item.addToLayout(self->pageParent, self->formLayout);
				item.connectTo(page);
			});
		}

		template <typename T>
		auto& get() {
			return std::get<T>(fields);
		}

		template <typename ValueType>
		void attach(ValueType& value) {
			each([&value = value](auto& item) { item.attach(value); });
		}

		template <typename ValueType>
		void readValue(ValueType& value) {
			each([&value = value](auto& item) { item.readValue(value); });
		}

		bool isValid() {
			return logical_and([](auto& item) { return item.isValid(); });
		}
		void restyleFields() {
			auto const lightMode = LineEditBase::isLightMode();
			each([lightMode](auto& item) { item.restyleField(lightMode); });
		}

	protected:
		template <typename Cb>
		void each(Cb&& cb) {
			each_impl(std::forward<Cb>(cb), std::index_sequence_for<Fields...>{});
		}

		template <typename Cb>
		bool logical_and(Cb&& cb) {
			return and_impl(std::forward<Cb>(cb), std::index_sequence_for<Fields...>{});
		}

	private:
		template <size_t... Indexes, typename Cb>
		void each_impl(Cb&& cb, std::index_sequence<Indexes...>) {
			(cb(std::get<Indexes>(fields)), ...);
		}
		template <size_t... Indexes, typename Cb>
		bool and_impl(Cb&& cb, std::index_sequence<Indexes...>) {
			return (cb(std::get<Indexes>(fields)) && ...);
		}
	};
}  // namespace quick_dra::gui
