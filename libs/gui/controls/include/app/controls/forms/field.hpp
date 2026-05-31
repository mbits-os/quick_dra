// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <app/controls/forms/base.hpp>
#include <quick_dra/lex/validators.hpp>
#include <quick_dra/models/types.hpp>

namespace quick_dra::gui {
	namespace detail {
		template <typename Suite, typename Target, std::invocable<Target&> auto Lambda>
		struct FieldBase {
			using target_type = Target;
			using value_type = std::remove_cvref_t<decltype(Lambda(std::declval<Target&>()))>;
			using validation_suite = Suite;

			static Validation validate(std::string_view value) {
				auto const length = value.length();
				return length < Suite::min_length()   ? length == 0 ? Validation::Empty : Validation::TooShort
				       : length > Suite::max_length() ? Validation::TooLong
				       : Suite::is_valid(value)       ? Validation::Ok
				                                      : Validation::Invalid;
			}

			static auto& getField(Target& payer) { return Lambda(payer); }
		};

		struct NonEmptyValidator {
			static size_t min_length() noexcept { return 1; }
			static size_t max_length() noexcept { return std::numeric_limits<size_t>::max(); }
			static bool is_valid(std::string_view id) noexcept { return !id.empty(); }
		};

		struct InsuranceTitleValidator {
			static size_t min_length() noexcept { return 8; }
			static size_t max_length() noexcept { return 8; }
			static bool is_valid(std::string_view id) noexcept {
				insurance_title title{};
				return insurance_title::parse(id, title);
			}
		};

		struct NullValidator {
			static size_t min_length() noexcept { return 0; }
			static size_t max_length() noexcept { return std::numeric_limits<size_t>::max(); }
			static bool is_valid(std::string_view) noexcept { return true; }
		};
	}  // namespace detail

	struct insured_type : person {
		struct history_type {
			std::chrono::year_month since{};
			ratio part_time_scale{};
			currency salary{};
			constexpr auto operator<=>(history_type const&) const noexcept = default;
		};
		insurance_title title;
		std::optional<std::string> social_id;
		std::vector<history_type> history;
		constexpr auto operator<=>(insured_type const&) const noexcept = default;
	};

#define DECLARE_FIELD(NAME, VALIDATOR, TARGET, MEMBER) \
	struct NAME : detail::FieldBase<VALIDATOR, TARGET, [](TARGET& p) -> auto& { return p.MEMBER; }>
#define ERROR_CHECKSUM_NEEDED(LABEL)          \
	static constexpr auto label = LABEL ""sv; \
	static constexpr auto error_message = LABEL " musi mieć poprawną sumę kontrolną"sv;
#define ENUM_KEY(ID) static constexpr auto enum_key = #ID ""sv;

	DECLARE_FIELD(FirstNameDeclaration, detail::NonEmptyValidator, person, first_name) {
		static constexpr auto label = "Imię pierwsze"sv;
		static constexpr auto error_message = ""sv;
	};

	DECLARE_FIELD(LastNameDeclaration, detail::NonEmptyValidator, person, last_name) {
		static constexpr auto label = "Nazwisko"sv;
		static constexpr auto error_message = ""sv;
	};

	DECLARE_FIELD(TitleDeclaration, detail::InsuranceTitleValidator, insured_type, title) {
		static constexpr auto label = "Kod tytułu ubezpieczenia"sv;
		static constexpr auto error_message =
		    "Tytuł ubezpieczenia powinien składać się z 6 cyfr w formacie \"0000 0 0\""sv;
	};

	DECLARE_FIELD(TaxIdDeclaration, tax_id_validator, payer_t, tax_id) {
		static constexpr auto label = "Numer NIP"sv;
		static constexpr auto error_message =
		    "Numer NIP musi mieć poprawną sumę kontrolną i nie może zawierać kresek"sv;
	};

	DECLARE_FIELD(EmploymentHistoryDeclaration, detail::NullValidator, insured_type, history) {
		static constexpr auto label = "Historia zatrudnienia"sv;
		static constexpr auto error_message = ""sv;
	};

	DECLARE_FIELD(EmploymentHistorySinceDeclaration, detail::NullValidator, insured_type::history_type, since) {
		static constexpr auto label = "Początek"sv;
		static constexpr auto error_message = ""sv;
		static constexpr auto unique = true;

		/// returns next month
		static year_month newRow() {
			auto const today = get_today();
			return today.year() / today.month() + months{1};
		}
	};

	DECLARE_FIELD(EmploymentHistoryPartTimeScaleDeclaration,
	              detail::NullValidator,
	              insured_type::history_type,
	              part_time_scale) {
		static constexpr auto label = "Część"sv;
		static constexpr auto error_message = ""sv;
		static constexpr auto unique = false;

		static constexpr ratio newRow() { return {1, 1}; }
	};

	DECLARE_FIELD(EmploymentHistorySalaryDeclaration, detail::NullValidator, insured_type::history_type, salary) {
		static constexpr auto label = "Pensja"sv;
		static constexpr auto error_message = ""sv;
		static constexpr auto unique = false;

		static constexpr currency newRow() { return minimal_salary; }
	};

	template <typename Target>
	DECLARE_FIELD(SocialIdDeclarationBase, social_id_validator, Target, social_id) {
		ERROR_CHECKSUM_NEEDED("Numer PESEL");
	};

	struct SocialIdDeclaration : SocialIdDeclarationBase<payer_t> {};

	struct SocialIdEnumDeclaration : SocialIdDeclarationBase<insured_type> {
		ENUM_KEY(P);
	};

	DECLARE_FIELD(IdCardEnumDeclaration, id_card_validator, person, id_card) {
		ENUM_KEY(1);
		ERROR_CHECKSUM_NEEDED("Dowód osobisty");
	};

	DECLARE_FIELD(PassportEnumDeclaration, pl_passport_validator, person, passport) {
		ENUM_KEY(2);
		ERROR_CHECKSUM_NEEDED("Paszport");
	};
}  // namespace quick_dra::gui
