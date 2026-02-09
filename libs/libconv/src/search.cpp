// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <quick_dra/base/str.hpp>
#include <quick_dra/conv/search.hpp>

namespace quick_dra {
	namespace {
		auto make_records(std::span<partial::insured_t> const& insured) {
			std::vector<std::vector<std::string>> records{};
			records.reserve(insured.size());
			for (auto const& person : insured) {
				records.emplace_back();
				auto& dst = records.back();
				dst.reserve(3);
				for (auto const& field :
				     {person.document, person.first_name, person.last_name}) {
					auto const key = field.transform(
					    [](auto const& fld) { return to_upper(fld); });
					if (key) {
						dst.push_back(std::move(*key));
					}
				}
			}

			return records;
		}

		void search_insured_from_records(
		    std::vector<unsigned>& found,
		    std::vector<std::vector<std::string>> const& records,
		    auto const& predicate) {
			unsigned index = 0;

			for (auto const& record : records) {
				auto matching = false;
				for (auto const& field : record) {
					if (predicate(field)) {
						matching = true;
						break;
					}
				}

				if (matching) {
					found.push_back(index);
				}

				++index;
			}
		}
	}  // namespace

	std::vector<unsigned> search_insured_from_position(
	    unsigned position,
	    std::span<partial::insured_t> const& insured,
	    std::function<void(std::string const&)> const& on_error) {
		if (position < 1 || position > insured.size()) {
			if (insured.size() == 1) {
				on_error("argument --pos must be equal to 1"s);
			} else {
				on_error(
				    fmt::format("argument --pos must be between 1 and "
				                "{}, inclusive",
				                insured.size()));
			}
		}

		return {position - 1};
	}

	std::vector<unsigned> search_insured_from_keyword(
	    std::string_view search_keyword,
	    std::span<partial::insured_t> const& insured,
	    std::function<void(std::string const&)> const& on_error) {
		std::vector<unsigned> result{};
		auto const records = make_records(insured);

		auto const upper = to_upper(search_keyword);
		auto const view = std::string_view{upper};
		search_insured_from_records(
		    result, records,
		    [view](std::string const& field) { return field == view; });

		if (result.empty()) {
			search_insured_from_records(
			    result, records, [view](std::string const& field) {
				    return field.starts_with(view) || field.ends_with(view);
			    });
		}

		if (result.empty()) {
			on_error(fmt::format("--find: could not find any record using `{}'",
			                     search_keyword));
		}

		return result;
	}
}  // namespace quick_dra
