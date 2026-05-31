// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QEvent>
#include <app/controls/PageScrollArea.hpp>
#include <app/pages/PayerEditPage.hpp>
#include <app/utils/forms.hpp>
#include <quick_dra/lex/validators.hpp>
#include <string>

namespace quick_dra::gui {
	namespace {
		payer_t payer_or_empty(partial::payer_t const& from) {
			return {
			    {
			        .last_name = from.last_name.value_or(""s),
			        .id_card = {},
			        .passport = {},
			        .first_name = from.first_name.value_or(""s),
			        .kind = from.kind.value_or(""s),
			        .document = from.document.value_or(""s),
			    },
			    from.tax_id.value_or(""s),
			    from.social_id.value_or(""s),
			};
		}

		std::optional<std::string> relax_string(std::string_view value) {
			auto const val = strip_sv(value);
			if (val.empty()) return std::nullopt;
			return std::string{value.data(), value.size()};
		}

		partial::payer_t relax_payer(payer_t const& from) {
			return {
			    {
			        .last_name = relax_string(from.last_name),
			        .id_card = {},
			        .passport = {},
			        .first_name = relax_string(from.first_name),
			        .kind = relax_string(from.kind),
			        .document = relax_string(from.document),
			    },
			    relax_string(from.tax_id),
			    relax_string(from.social_id),
			};
		}
	}  // namespace

	void PayerEditPage::UI::setupPageUI(PayerEditPage* page) {
		auto const [_, pageParentPtr] = PageScrollArea::setupPage(page);
		pageParent = pageParentPtr;
		formLayout = new QFormLayout(pageParent);
		each([self = this, page](auto& item) {
			item.addToLayout(self->pageParent, self->formLayout);
			item.connectTo(page);
		});
	}

	PayerEditPage::PayerEditPage(QWidget* parent) : PagedWidget{parent} { ui.setupPageUI(this); }

	PayerEditPage::~PayerEditPage() = default;

	bool PayerEditPage::event(QEvent* event) {
		if (event->type() == QEvent::PaletteChange) restyleFields();
		return PagedWidget::event(event);
	}

	void PayerEditPage::connectPage() {
		auto& payer = *globals().data().cfg.payer;
		currentValue = acceptedValue = payer_or_empty(payer);
		setWindowTitle(QString{"%1 (Płatnik)"}.arg(name_from(payer.first_name, payer.last_name, false)));
		ui.each([&currentValue = currentValue](auto& item) { item.attach(currentValue); });
	}

	void PayerEditPage::accept() {
		globals().storePayer(relax_payer(currentValue));
		acceptedValue = currentValue;
		setFormDirty(false);
		leavePage();
	}

	void PayerEditPage::updateCurrentValue() {
		ui.each([&currentValue = this->currentValue](auto& item) { item.readValue(currentValue); });
		auto const name = name_from(relax_string(currentValue.first_name), relax_string(currentValue.last_name), false);
		setWindowTitle(QString{"%1 (Płatnik)"}.arg(QString::fromUtf8(name)));
		setFormDirty(currentValue != acceptedValue);
	}

	void PayerEditPage::updateCurrentIsValid() {
		auto const valid = ui.logical_and([](auto& item) { return item.isValid(); });
		setFormValid(valid);
	}

	void PayerEditPage::updateValue(QString const& value, std::string& target) {
		auto bytes = value.toUtf8();
		target.assign(bytes);
		setFormDirty(currentValue != acceptedValue);
	}

	void PayerEditPage::restyleFields() {
		auto const lightMode = LineEditBase::isLightMode();
		ui.each([lightMode](auto& item) { item.restyleField(lightMode); });
	}
}  // namespace quick_dra::gui
