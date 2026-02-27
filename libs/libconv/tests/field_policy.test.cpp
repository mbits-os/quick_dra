// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <fmt/std.h>
#include <gtest/gtest.h>
#include <concepts>
#include <quick_dra/base/str.hpp>
#include <quick_dra/base/types.hpp>
#include <quick_dra/conv/field_policy.hpp>
#include <quick_dra/conv/validators.hpp>
#include <sstream>

namespace quick_dra::testing {
	using std::literals::operator""s;
	using std::literals::operator""sv;

	template <typename Subject>
	struct conversation {
		using value_type = Subject;
		bool ask_questions{true};
		value_type opts{};
		value_type dst{};

		std::istream* cin = &std::cin;
		void attach(std::istream& in) { cin = &in; }
	};

	struct creature {
		std::optional<std::string> name;
		std::optional<std::string> species;
		std::optional<unsigned> age;
	};

	namespace getters {
		struct name {
			using person_type = creature;
			inline std::optional<std::string>& operator()(
			    person_type& person) const noexcept {
				return person.name;
			}
		};

		struct species {
			using person_type = creature;
			inline std::optional<std::string>& operator()(
			    person_type& person) const noexcept {
				return person.species;
			}
		};

		struct age {
			using person_type = creature;
			inline std::optional<unsigned>& operator()(
			    person_type& person) const noexcept {
				return person.age;
			}
		};
	}  // namespace getters

	static constexpr auto lab_sel =
	    "Creature species"_label / getters::species{};
	static constexpr auto species =
	    lab_sel / builtin::policies::always_valid<std::string>;
	static constexpr auto name = "Name"_label / getters::name{} /
	                             builtin::policies::always_valid<std::string>;
	static constexpr auto age =
	    "Age"_label / getters::age{} /
	    [](std::string&& val, std::optional<unsigned>& dst, bool) {
		    unsigned result{};
		    if (!from_chars(val, result)) return false;
		    dst = result;
		    return true;
	    };

	template <typename A, typename S>
	using policy_type =
	    field_policy<A, S, policy_builder::validator_function<A>>;

	template <typename A, typename S>
	using field_answer_type = interactive::field_answer<A, policy_type<A, S>>;

	template <typename FieldPolicy, typename Person>
	bool validate_and_update(FieldPolicy const& policy,
	                         Person& dst,
	                         std::string&& value) {
		return policy.get_validator()(std::move(value), policy.select(dst),
		                              false);
	}

	TEST(field_policy, builder) {
		auto spock = creature{"Spock"s, "Volcan"s, std::nullopt};
		auto const& spock_cref = spock;

		EXPECT_EQ(label("Text for user"sv), "Text for user"_label);
		EXPECT_TRUE((std::same_as<decltype(lab_sel),
		                          policy_builder::label_selector<
		                              std::string, getters::species> const>));
		EXPECT_TRUE(
		    (std::same_as<decltype(species),
		                  policy_type<std::string, getters::species> const>));
		EXPECT_EQ(lab_sel.value, "Creature species"sv);
		EXPECT_EQ(lab_sel.selector()(spock), "Volcan"sv);
		EXPECT_EQ(species.label, "Creature species"sv);
		EXPECT_EQ(name.select(spock_cref), "Spock"sv);
		EXPECT_EQ(species.select(spock), "Volcan"sv);
		// Kelvin timeline
		EXPECT_TRUE(validate_and_update(name, spock, "Abassador Spock"s));
		EXPECT_TRUE(validate_and_update(species, spock, "Human"s));
		EXPECT_TRUE(validate_and_update(age, spock, "161"s));
		EXPECT_EQ(spock.name, "Abassador Spock"sv);
		EXPECT_EQ(spock.species, "Human"sv);
		EXPECT_EQ(spock.age, 161u);
	}

	TEST(field_policy, get_field_answer) {
		auto const answer = species.get_field_answer();

		conversation<creature> conv{};
		conv.dst = creature{"Spock"s, "Volcan"s, std::nullopt};
		conv.opts = creature{std::nullopt, "Human"s, std::nullopt};
		conv.ask_questions = false;

		EXPECT_TRUE((std::same_as<
		             decltype(answer),
		             field_answer_type<std::string, getters::species> const>));

		std::istringstream in{""s};
		conv.attach(in);
		EXPECT_TRUE(answer.get_answer(conv));
		EXPECT_EQ(conv.dst.species, "Human"sv);
	}

	TEST(field_policy, partial_policies) {
		partial::payer_t payer{};
		partial::insured_t insured{};

#define EXPECT_POLICY(TARGET, POLICY, VALUE, VALID, EXPECTED)          \
	{                                                                  \
		EXPECT_##VALID(validate_and_update(POLICY, TARGET, VALUE##s)); \
		EXPECT_EQ(POLICY.select(TARGET), EXPECTED);                    \
	}
#define EXPECT_POLICY_VALID(TARGET, POLICY, VALUE) \
	EXPECT_POLICY(TARGET, POLICY, VALUE, TRUE, VALUE##sv)
#define EXPECT_POLICY_VALID_(TARGET, POLICY, VALUE, EXPECTED) \
	EXPECT_POLICY(TARGET, POLICY, VALUE, TRUE, EXPECTED)
#define EXPECT_POLICY_INVALID(TARGET, POLICY, VALUE) \
	EXPECT_POLICY(TARGET, POLICY, VALUE, FALSE, std::nullopt)

		EXPECT_POLICY_VALID(payer, builtin::policies::last_name, "Last");
		EXPECT_POLICY_VALID(payer, builtin::policies::first_name, "First");
		EXPECT_POLICY_VALID(payer, builtin::policies::id_card, "AAA000000");
		EXPECT_POLICY_VALID(payer, builtin::policies::passport, "AA0000000");
		EXPECT_POLICY_VALID(payer, builtin::policies::kind, "K");
		EXPECT_POLICY_VALID(payer, builtin::policies::document, "always-valid");
		EXPECT_POLICY_VALID(payer, builtin::policies::tax_id, "7680002466");
		EXPECT_POLICY_VALID(payer, builtin::policies::social_id, "26211012346");

		EXPECT_POLICY_VALID(insured, builtin::policies::last_name, "Last");
		EXPECT_POLICY_VALID(insured, builtin::policies::first_name, "First");
		EXPECT_POLICY_VALID(insured, builtin::policies::id_card, "AAA000000");
		EXPECT_POLICY_INVALID(insured, builtin::policies::passport,
		                      "AA1000000");
		EXPECT_POLICY_VALID(insured, builtin::policies::social_id,
		                    "26211012346");
		EXPECT_POLICY_VALID(insured, builtin::policies::kind, "K");
		EXPECT_POLICY_VALID(insured, builtin::policies::document,
		                    "always-valid");
		EXPECT_POLICY_INVALID(insured, builtin::policies::title, "0110 A B");
		EXPECT_POLICY_INVALID(insured, builtin::policies::part_time_scale,
		                      "345/B");
		EXPECT_POLICY_VALID_(insured, builtin::policies::salary, "minimal",
		                     minimal_salary);
	}

	TEST(field_policy, enums) {
		auto const social_id_item = get_enum_item(builtin::policies::social_id);
		auto const passport_item = get_enum_item(builtin::policies::passport);

		static_assert(
		    interactive::details::Enumerator<decltype(social_id_item)>);
		static_assert(
		    interactive::details::Enumerator<decltype(passport_item)>);

		EXPECT_EQ(std::get<0>(social_id_item.get_item()), 'P');
		EXPECT_EQ(std::get<1>(social_id_item.get_item()), "PESEL"sv);
		EXPECT_EQ(std::get<0>(passport_item.get_item()), '2');
		EXPECT_EQ(std::get<1>(passport_item.get_item()), "Passport"sv);

		{
			auto kind_enum = builtin::policies::kind.get_enum_field(
			    get_enum_item(builtin::policies::social_id),
			    get_enum_item(builtin::policies::passport));
			std::istringstream in{""s};
			conversation<partial::insured_t> conv{.cin = &in};
			conv.opts.passport = "AA0000000"s;
			conv.ask_questions = false;

			EXPECT_TRUE(kind_enum.get_answer(conv));
			EXPECT_EQ(conv.dst.kind, "2"s);
		}

		{
			auto kind_enum = builtin::policies::kind.get_enum_field(
			    get_enum_item(builtin::policies::social_id),
			    get_enum_item(builtin::policies::passport));
			std::istringstream in{"\n"s};
			conversation<partial::insured_t> conv{.cin = &in};
			conv.opts.passport = "AA0000000"s;
			conv.ask_questions = true;

			::testing::internal::CaptureStdout();
			auto const result = kind_enum.get_answer(conv);
			auto const log = ::testing::internal::GetCapturedStdout();

			EXPECT_TRUE(result);
			EXPECT_EQ(conv.dst.kind, "2"s);
			EXPECT_EQ(
			    log,
			    "\033[0;36mDocument kind\033[0;90m [P - PESEL, [2] - Passport]\033[m> "sv);
		}

		{
			auto kind_enum = builtin::policies::kind.get_enum_field(
			    get_enum_item(builtin::policies::social_id),
			    get_enum_item(builtin::policies::passport));
			std::istringstream in{"2\n"s};

			conversation<partial::insured_t> conv{.cin = &in};
			conv.ask_questions = true;

			::testing::internal::CaptureStdout();
			auto const result = kind_enum.get_answer(conv);
			auto const log = ::testing::internal::GetCapturedStdout();

			EXPECT_TRUE(result);
			EXPECT_EQ(conv.dst.kind, "2"s);
			EXPECT_EQ(
			    log,
			    "\033[0;36mDocument kind\033[0;90m [P - PESEL, 2 - Passport]\033[m> "sv);
		}

		{
			auto kind_enum = builtin::policies::kind.get_enum_field(
			    get_enum_item(builtin::policies::social_id),
			    get_enum_item(builtin::policies::passport));
			std::istringstream in{""s};

			conversation<partial::insured_t> conv{.cin = &in};
			conv.ask_questions = true;

			::testing::internal::CaptureStdout();
			auto const result = kind_enum.get_answer(conv);
			auto const log = ::testing::internal::GetCapturedStdout();

			EXPECT_FALSE(result);
			EXPECT_FALSE(conv.dst.kind);
			EXPECT_EQ(
			    log,
			    "\033[0;36mDocument kind\033[0;90m [P - PESEL, 2 - Passport]\033[m> "sv);
		}

		{
			auto kind_enum = builtin::policies::kind.get_enum_field(
			    get_enum_item(builtin::policies::social_id),
			    get_enum_item(builtin::policies::passport));
			std::istringstream in{""s};

			conversation<partial::insured_t> conv{.cin = &in};
			conv.ask_questions = false;

			::testing::internal::CaptureStdout();
			auto const result = kind_enum.get_answer(conv);
			auto const log = ::testing::internal::GetCapturedStdout();

			EXPECT_FALSE(result);
			EXPECT_FALSE(conv.dst.kind);
			EXPECT_EQ(log, ""sv);
		}
	}

}  // namespace quick_dra::testing
