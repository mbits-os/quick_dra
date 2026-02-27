// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <fmt/std.h>
#include <gtest/gtest.h>
#include <array>
#include <concepts>
#include <quick_dra/base/str.hpp>
#include <quick_dra/conv/search.hpp>
#include <sstream>
#include <tuple>
#include <variant>

namespace std {
	template <typename T>
	bool operator==(std::span<T> const& lhs, std::span<T> const& rhs) noexcept {
		if (!(lhs.size() == rhs.size())) return false;
		for (size_t index = 0; index < lhs.size(); ++index) {
			if (!(lhs[index] == rhs[index])) return false;
		}
		return true;
	}
}  // namespace std

namespace quick_dra::testing {
	using std::literals::operator""s;
	using std::literals::operator""sv;

	using search_term_view = std::variant<unsigned, std::string_view>;
	using testcase = std::
	    tuple<search_term_view, std::span<unsigned const>, std::string_view>;

	struct person {
		std::string_view first_name;
		std::string_view last_name;
		std::string_view kind;
		std::string_view document;
	};

	static constexpr person people[] = {
	    {"Piotr"sv, "Iksiński"sv, "1"sv, "ABC523456"sv},
	    {"Jan"sv, "Iksiński"sv, "2"sv, "EH0123456"sv},
	    {"Maria"sv, "Iksińska"sv, "P"sv, "26211012346"sv},
	};

	class search : public ::testing::TestWithParam<testcase> {
	public:
		void test_lookup(testcase const& param,
		                 std::span<partial::insured_t> const& insured) {
			auto const& [term_view, expected_indexes_span, expected_output] =
			    param;
			struct conv {
				search_term operator()(unsigned index) const { return index; }
				search_term operator()(std::string_view term) const {
					return as_str(term);
				}
				search_term operator()(search_term_view const& term) const {
					return std::visit(*this, term);
				}
			};
			auto const term = conv{}(term_view);
			std::vector<unsigned> expected_indexes{
			    expected_indexes_span.begin(), expected_indexes_span.end()};

			std::string log{};
			struct carry_on {};

			std::vector<unsigned> actual{};
			try {
				actual = search_insured_from_term(
				    term, insured, [&log](std::string const& err) {
					    log = err;
					    throw carry_on{};
				    });
			} catch (carry_on const&) {
				// pass
			}

			EXPECT_EQ(actual, expected_indexes);
			EXPECT_EQ(log, expected_output);
		}
	};

	TEST_P(search, lookup) {
		std::vector<partial::insured_t> insured{};
		insured.reserve(std::size(people));
		std::transform(std::begin(people), std::end(people),
		               std::back_inserter(insured), [](auto const& person) {
			               return partial::insured_t{
			                   partial::person{
			                       .last_name = as_str(person.last_name),
			                       .id_card = std::nullopt,
			                       .passport = std::nullopt,
			                       .first_name = as_str(person.first_name),
			                       .kind = as_str(person.kind),
			                       .document = as_str(person.document),
			                   },
			                   std::nullopt,
			                   std::nullopt,
			                   std::nullopt,
			                   std::nullopt,
			               };
		               });

		test_lookup(GetParam(), insured);
	}

	static consteval search_term_view with(unsigned id) { return id; };
	static consteval search_term_view with(std::string_view id) { return id; };

	static constexpr auto none = std::array<unsigned, 0>{};
	static constexpr unsigned one[] = {0};
	static constexpr unsigned two[] = {1};
	static constexpr unsigned three[] = {2};
	static constexpr unsigned everybody[] = {0, 1, 2};

	static constexpr testcase tests[] = {
	    {with(1), one, ""sv},
	    {with(5), none, "argument --pos must be between 1 and 3, inclusive"sv},
	    {with("iksiń"), everybody, ""sv},
	    {with("eh012"), two, ""sv},
	    {with("maria"), three, ""sv},
	    {with("jean-luc"), none,
	     "--find: could not find any record using `jean-luc'"sv},
	};

	INSTANTIATE_TEST_SUITE_P(test, search, ::testing::ValuesIn(tests));

	TEST_F(search, pos_lookup_in_short_list) {
		std::vector<partial::insured_t> insured{};
		test_lookup({with(2), none, "insured list is empty"sv}, insured);

		insured.push_back({});
		test_lookup({with(2), none, "argument --pos must be equal to 1"sv},
		            insured);
	}
}  // namespace quick_dra::testing
