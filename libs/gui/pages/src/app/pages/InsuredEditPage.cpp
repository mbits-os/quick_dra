// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QEvent>
#include <app/controls/PageScrollArea.hpp>
#include <app/pages/InsuredEditPage.hpp>
#include <app/pages/RemoveHistoryPage.hpp>
#include <app/utils/forms.hpp>
#include <quick_dra/lex/validators.hpp>

namespace quick_dra::gui {
	namespace {
		insured_type::history_type history_conv(std::pair<year_month, partial::employment_history> const& from) {
			return {
			    .since = from.first,
			    .part_time_scale = from.second.part_time_scale.value_or(ratio{1, 1}),
			    .salary = from.second.salary.value_or(minimal_salary),
			};
		}

		std::pair<year_month, partial::employment_history> history_conv(insured_type::history_type const& from) {
			return {
			    from.since,
			    {
			        .part_time_scale =
			            from.part_time_scale == ratio{1, 1} ? std::nullopt : std::optional{from.part_time_scale},
			        .salary = from.salary == minimal_salary ? std::nullopt : std::optional{from.salary},
			    },
			};
		}

		std::map<std::chrono::year_month, partial::employment_history> relax_history(
		    std::vector<insured_type::history_type> const& values) {
			std::map<std::chrono::year_month, partial::employment_history> result{};
			for (auto const& item : values) {
				auto pair = history_conv(item);
				auto it = result.lower_bound(pair.first);
				result.insert(it, pair);
			}
			return result;
		}

		insured_type insured_or_empty(partial::insured_t const& from) {
			using history_type = std::vector<insured_type::history_type>;
			return {
			    {
			        .last_name = from.last_name.value_or(""s),
			        .id_card = {},
			        .passport = {},
			        .first_name = from.first_name.value_or(""s),
			        .kind = from.kind.value_or(""s),
			        .document = from.document.value_or(""s),
			    },
			    from.title.value_or(insurance_title{}),
			    {},
			    from.history
			        .transform([](auto const& values) {
				        history_type result{};
				        result.reserve(values.size());
				        std::transform(values.begin(), values.end(), std::back_inserter(result),
				                       [](auto const& item) { return history_conv(item); });

				        return result;
			        })
			        .value_or(history_type{}),
			};
		}

		std::optional<std::string> relax_string(std::string_view value) {
			auto const val = strip_sv(value);
			if (val.empty()) return std::nullopt;
			return std::string{value.data(), value.size()};
		}

		partial::insured_t relax_insured(insured_type const& from) {
			return {
			    {
			        .last_name = relax_string(from.last_name),
			        .id_card = {},
			        .passport = {},
			        .first_name = relax_string(from.first_name),
			        .kind = relax_string(from.kind),
			        .document = relax_string(from.document),
			    },
			    from.title,
			    {},
			    relax_history(from.history),
			};
		}

		std::optional<std::string> find_duplicate(std::string_view kind,
		                                          std::string_view document,
		                                          size_t pos,
		                                          std::vector<partial::insured_t> const& insured) {
			unsigned index = 0;
			for (auto const& person : insured) {
				if (index == pos) {
					++index;
					continue;
				}

				if (person.kind == kind && person.document == document) {
					break;
				}

				++index;
			}

			if (index == insured.size()) {
				return {};
			}

			auto const& item = insured[index];
			return name_from(item.first_name, item.last_name, false);
		}
	}  // namespace

	void InsuredEditPage::UI::setupPageUI(InsuredEditPage* page) {
		auto const [_, pageParentPtr] = PageScrollArea::setupPage(page);
		pageParent = pageParentPtr;
		formLayout = new QFormLayout(pageParent);
		each([self = this, page](auto& item) {
			item.addToLayout(self->pageParent, self->formLayout);
			item.connectTo(page);
		});
		QObject::connect(history.removeButton, &QPushButton::clicked, page,
		                 &InsuredEditPage::removeEmploymentHistoryEntries);
		QObject::connect(history.addButton, &QPushButton::clicked, page,
		                 &InsuredEditPage::addNewEmploymentHistoryEntry);
	}

	InsuredEditPage::InsuredEditPage(size_t index, QWidget* parent) : PagedWidget{parent}, insuredIndex{index} {
		ui.setupPageUI(this);
	}

	InsuredEditPage::~InsuredEditPage() = default;

	bool InsuredEditPage::event(QEvent* event) {
		if (event->type() == QEvent::PaletteChange) restyleFields();
		return PagedWidget::event(event);
	}

	void InsuredEditPage::connectPage() {
		auto& insured = *globals().data().cfg.insured;
		if (insuredIndex >= insured.size()) {
			currentValue = acceptedValue = {};
			currentValue.title.title_code = acceptedValue.title.title_code = "0000"sv;
			setWindowTitle(QString{"%1 (Ubezpieczony)"}.arg(name_from(std::nullopt, std::nullopt, false)));
		} else {
			auto& ref = insured[insuredIndex];
			currentValue = acceptedValue = insured_or_empty(ref);
			setWindowTitle(QString{"%1 (Ubezpieczony)"}.arg(name_from(ref.first_name, ref.last_name, false)));
		}
		ui.document.setBlockChecker(
		    [insuredIndex = insuredIndex, &insured](std::string_view kind, std::string_view number) {
			    return find_duplicate(kind, number, insuredIndex, insured);
		    });
		ui.each([&currentValue = currentValue](auto& item) { item.attach(currentValue); });
	}

	void InsuredEditPage::accept() {
		globals().storeInsured(insuredIndex, relax_insured(currentValue));
		acceptedValue = currentValue;
		setFormDirty(false);
		leavePage();
	}

	void InsuredEditPage::updateCurrentValue() {
		ui.each([&currentValue = this->currentValue](auto& item) { item.readValue(currentValue); });
		auto const name = name_from(relax_string(currentValue.first_name), relax_string(currentValue.last_name), false);
		setWindowTitle(QString{"%1 (Ubezpieczony)"}.arg(QString::fromUtf8(name)));
		setFormDirty(currentValue != acceptedValue);
	}

	void InsuredEditPage::updateCurrentIsValid() {
		auto const valid = ui.logical_and([](auto& item) { return item.isValid(); });
		setFormValid(valid);
	}

	void InsuredEditPage::removeEmploymentHistoryEntries() {
		stack().push<RemoveHistoryPage>(currentValue.history,
		                                [self = this](auto&& history) { self->storeNewHistory(std::move(history)); });
	}

	void InsuredEditPage::storeNewHistory(std::vector<insured_type::history_type>&& history) {
		using Model = typename decltype(ui.history)::Model;
		auto const model = static_cast<Model*>(ui.history.view->model());
		model->replaceRows(std::move(history));
		setFormDirty(currentValue != acceptedValue);
	}

	void InsuredEditPage::addNewEmploymentHistoryEntry() {
		using Model = typename decltype(ui.history)::Model;
		auto const model = static_cast<Model*>(ui.history.view->model());
		model->addNewRow();
		setFormDirty(currentValue != acceptedValue);
	}

	void InsuredEditPage::updateValue(QString const& value, std::string& target) {
		auto bytes = value.toUtf8();
		target.assign(bytes);
		setFormDirty(currentValue != acceptedValue);
	}

	void InsuredEditPage::restyleFields() {
		auto const lightMode = LineEditBase::isLightMode();
		ui.each([lightMode](auto& item) { item.restyleField(lightMode); });
	}
}  // namespace quick_dra::gui
