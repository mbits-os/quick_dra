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
		static constexpr name_from_config payer_title{
		    .format = unknown_name::symbolic,
		    .format_for = name_hint::payer,
		};

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

	PayerEditPage::PayerEditPage(QWidget* parent) : PagedWidget{parent} { ui.setupPageUI(this); }

	PayerEditPage::~PayerEditPage() = default;

	bool PayerEditPage::event(QEvent* event) {
		if (event->type() == QEvent::PaletteChange) {
			ui.restyleFields();
		}
		return PagedWidget::event(event);
	}

	void PayerEditPage::connectPage() {
		auto& payer = *globals().data().cfg.payer;
		currentValue = acceptedValue = payer_or_empty(payer);
		setWindowTitle(
		    QString{"%1 (Płatnik)"}.arg(QString::fromUtf8(name_from(payer.first_name, payer.last_name, payer_title))));
		ui.attach(currentValue);
	}

	void PayerEditPage::accept() {
		globals().storePayer(relax_payer(currentValue));
		acceptedValue = currentValue;
		setFormDirty(false);
		leavePage();
	}

	void PayerEditPage::updateCurrentValue() {
		ui.readValue(currentValue);
		auto const name =
		    name_from(relax_string(currentValue.first_name), relax_string(currentValue.last_name), payer_title);
		setWindowTitle(QString{"%1 (Płatnik)"}.arg(QString::fromUtf8(name)));
		setFormDirty(currentValue != acceptedValue);
	}

	void PayerEditPage::updateFormValid() { setFormValid(ui.isValid()); }
}  // namespace quick_dra::gui
