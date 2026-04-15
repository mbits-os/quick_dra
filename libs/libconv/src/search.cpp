// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <quick_dra/base/str.hpp>
#include <quick_dra/conv/search.hpp>
#include <string>
#include <utility>
#include <vector>

namespace quick_dra {
	namespace {
		std::vector<std::string> make_record(partial::person const& person) {
			std::vector<std::string> result{};
			result.reserve(3);
			for (auto const& field : {person.document, person.first_name, person.last_name}) {
				auto const key = field.transform([](auto const& fld) { return to_upper(fld); });
				if (key) {
					result.push_back(std::move(*key));
				}
			}
			return result;
		}  // GCOV_EXCL_LINE[GCC]

		auto make_records(std::span<partial::insured_t> insured) {
			std::vector<std::vector<std::string>> records{};
			records.reserve(insured.size());
			for (auto const& person : insured) {
				records.emplace_back(make_record(person));
			}

			return records;
		}  // GCOV_EXCL_LINE[GCC]

		bool record_matches(std::vector<std::string> const& record, auto const& predicate) {
			for (auto const& field : record) {
				if (predicate(field)) {
					return true;
				}
			}
			return false;
		}

		void search_insured_from_records(std::vector<unsigned>& found,
		                                 std::vector<std::vector<std::string>> const& records,
		                                 auto const& predicate) {
			unsigned index = 0;

			for (auto const& record : records) {
				if (record_matches(record, predicate)) {
					found.push_back(index);
				}

				++index;
			}
		}
	}  // namespace

	match_level match_payer_from_keyword(std::string_view search_keyword, partial::payer_t const& payer) {
		auto const record = make_record(payer);

		auto const upper = to_upper(search_keyword);
		auto const view = std::string_view{upper};
		if (record_matches(record, [view](std::string const& field) {
			    // whole-string matching
			    return field == view;
		    })) {
			return match_level::direct;
		}

		if (record_matches(record, [view](std::string const& field) {
			    // partial matching
			    return field.starts_with(view) || field.ends_with(view);
		    })) {
			return match_level::partial;
		}

		return match_level::none;
	}

	std::vector<unsigned> search_insured_from_position(unsigned position,
	                                                   std::span<partial::insured_t> insured,
	                                                   std::function<void(std::string const&)> const& on_error) {
		if (position < 1 || position > insured.size()) {
			if (insured.empty()) {
				on_error("insured list is empty"s);
			} else if (insured.size() == 1) {
				on_error("argument --pos must be equal to 1"s);
			} else {  // GCOV_EXCL_LINE[WIN32]
				on_error(
				    fmt::format("argument --pos must be between 1 and "
				                "{}, inclusive",
				                insured.size()));
			}
		}

		return {position - 1};
	}

	std::vector<unsigned> search_insured_from_keyword(std::string_view search_keyword,
	                                                  std::span<partial::insured_t> insured,
	                                                  match_level payer,
	                                                  match_level* level,
	                                                  std::function<void(std::string const&)> const& on_error) {
		std::vector<unsigned> result{};
		auto const records = make_records(insured);

		auto const upper = to_upper(search_keyword);
		auto const view = std::string_view{upper};
		if (level) *level = match_level::direct;
		search_insured_from_records(result, records, [view](std::string const& field) { return field == view; });

		if (payer != match_level::direct && result.empty()) {
			if (level) *level = match_level::partial;
			search_insured_from_records(result, records, [view](std::string const& field) {
				return field.starts_with(view) || field.ends_with(view);
			});
		}

		if (result.empty()) {
			if (level) *level = match_level::none;
			if (payer == match_level::none) {
				on_error(fmt::format("--find: could not find any record using `{}'", search_keyword));
			}
		}

		return result;
	}
}  // namespace quick_dra
