// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <cmath>
#include <quick_dra/base/chrono.hpp>
#include <quick_dra/base/str.hpp>
#include <quick_dra/models/project_reader.hpp>
#include <utility>

namespace quick_dra {
	bool read_value(ref_ctx const& ref, percent& ctx) {
		if (!ref.ref().has_val()) {
			ctx = {};
			return false;
		}

		auto const value = ref.val();
		if (value.empty()) {
			ctx = {};
			return false;
		}

		if (!percent::parse(view(value), ctx)) {
			return ref.error("could not parse the percent value");
		}

		return true;
	}

	bool read_value(ref_ctx const& ref, currency& ctx) {
		if (!ref.ref().has_val()) {
			ctx = {};
			return false;
		}

		auto const value = ref.val();
		if (value.empty()) {
			ctx = {};
			return false;
		}

		if (!currency::parse(view(value), ctx)) {
			return ref.error("could not parse the currency value");
		}

		return true;
	}

	bool read_value(ref_ctx const& ref, ratio& ctx) {
		static constexpr auto expecting_NUM_DEN = "expecting N/M, e.g. 1/1 or 4/5"sv;

		if (!ref.ref().has_val()) {
			ctx = {};
			return false;
		}

		auto const value = ref.val();
		if (value.empty()) {
			ctx = {};
			return false;
		}

		auto const split = split_sv(view(value), '/'_sep);
		if (split.size() < 2) {
			ctx = {};
			return ref.error(expecting_NUM_DEN);
		}

		unsigned num{};
		unsigned den{};

		if (!convert_string(ref, split[0], num) || !convert_string(ref, split[1], den)) {
			return ref.error(expecting_NUM_DEN);
		}

		ctx = {.num = num, .den = den};
		return true;
	}

	bool read_value(ref_ctx const& ref, insurance_title& ctx) {
		if (!ref.ref().has_val()) {
			ctx = {};
			return false;
		}

		auto const value = ref.val();
		if (value.empty()) {
			ctx = {};
			return false;
		}

		if (!insurance_title::parse(view(value), ctx)) {
			return ref.error("could not parse the insurance title value");
		}

		return true;
	}

	bool read_value(ref_ctx const& ref, costs_of_obtaining& ctx) {
#define X(NAME) \
	if (!read_key(ref, #NAME, ctx.NAME)) return ref.error("while reading `" #NAME "`");

		X(local)
		X(remote)
#undef X
		return true;
	}

	bool read_value(ref_ctx const& ref, rate& ctx) {
#define X(NAME, KEY) \
	if (!read_key(ref, KEY, ctx.NAME, true)) return ref.error("while reading `" KEY "`");

		X(payer, "płatnik")
		X(insured, "ubezpieczony")
#undef X
		return true;
	}

	bool read_value(ref_ctx const& ref, rates& ctx) {
#define X(NAME, KEY) \
	if (!read_key(ref, KEY, ctx.NAME)) return ref.error("while reading `" KEY "`");
		CONTRIBUTIONS_EX(X)
#undef X
		return true;
	}

	void write_value(ryml::NodeRef& ref, currency const& ctx) { yaml::write_value(ref, fmt::format("{} zł", ctx)); }
	void write_value(ryml::NodeRef& ref, ratio const& ctx) {
		yaml::write_value(ref, fmt::format("{}/{}", ctx.num, ctx.den));
	}
	void write_value(ryml::NodeRef& ref, insurance_title const& ctx) {
		yaml::write_value(ref, fmt::format("{} {} {}", ctx.title_code, ctx.pension_right, ctx.disability_level));
	}

	bool convert_string(ref_ctx const& ref, c4::csubstr const& value, currency& ctx) {
		if (!currency::parse(view(value), ctx)) {
			return ref.error("could not parse the currency value");
		}

		return true;
	}

	void upgrade(v1::insured_t&& src, v2::insured_t& dst) {
		static_cast<v1::person&>(dst) = std::move(static_cast<v1::person&>(src));

		dst.title = std::move(src.title);
		dst.social_id = std::move(src.social_id);

		dst.history[null_month] = {
		    .part_time_scale = std::move(src.part_time_scale),
		    .salary = std::move(src.salary),
		};
	}

	void upgrade(v1::config&& src, v2::config& dst) {
		dst.version = src.version;
		dst.payer = std::move(src.payer);
		dst.insured.reserve(src.insured.size());
		for (auto& src_insured : src.insured) {
			dst.insured.emplace_back();
			upgrade(std::move(src_insured), dst.insured.back());
		}
	}

	void upgrade(v1::partial::insured_t&& src, v2::partial::insured_t& dst) {
		static_cast<v1::partial::person&>(dst) = std::move(static_cast<v1::partial::person&>(src));

		dst.title = std::move(src.title);
		dst.social_id = std::move(src.social_id);

		if (src.part_time_scale || src.salary) {
			dst.history.emplace();
			(*dst.history)[null_month] = {
			    .part_time_scale = std::move(src.part_time_scale),
			    .salary = std::move(src.salary),
			};
		}
	}

	void upgrade(v1::partial::config&& src, v2::partial::config& dst) {
		dst.version = std::move(src.version);
		dst.payer = std::move(src.payer);
		if (src.insured) {
			dst.insured.emplace();
			dst.insured->reserve(src.insured->size());
			for (auto& src_insured : *src.insured) {
				dst.insured->emplace_back();
				upgrade(std::move(src_insured), dst.insured->back());
			}
		}
	}

	void downgrade(v2::partial::insured_t const& src, v1::partial::insured_t& dst) {
		static_cast<v2::partial::person&>(dst) = std::move(static_cast<v2::partial::person const&>(src));

		dst.title = src.title;
		dst.social_id = src.social_id;
		dst.part_time_scale.reset();
		dst.salary.reset();

		if (src.history && !src.history->empty()) {
			auto const& src_employment = src.history->rbegin()->second;
			dst.part_time_scale = src_employment.part_time_scale;
			dst.salary = src_employment.salary;
		}
	}

	void downgrade(v2::partial::config const& src, v1::partial::config& dst) {
		dst.version = src.version;
		dst.payer = src.payer;
		if (src.insured) {
			dst.insured.emplace();
			dst.insured->reserve(src.insured->size());
			for (auto& src_insured : *src.insured) {
				dst.insured->emplace_back();
				downgrade(src_insured, dst.insured->back());
			}
		}
	}

}  // namespace quick_dra

namespace yaml {
	using quick_dra::operator""_sep;

	bool convert_string(ref_ctx const& ref, c4::csubstr const& value, std::chrono::year_month& ctx) {
		auto const val = view(value);
		if (!convert_string(val, ctx)) {
			return ref.error("expecting YYYY/MM or YYYY-MM"sv);
		}
		return true;
	}

	bool convert_string(std::string_view val, std::chrono::year_month& ctx) {
		auto split = split_sv(val, '/'_sep, 1);

		if (split.size() < 2) {
			split = split_sv(val, '-'_sep, 1);
		}

		if (split.size() < 2) {
			return false;
		}

		int year{};
		unsigned month{};

		if (!convert_string(split[0], year) || !convert_string(split[1], month)) {
			return false;
		}

		ctx = std::chrono::year{year} / static_cast<int>(month);

		return ctx.ok();
	}
}  // namespace yaml
