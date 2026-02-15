// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <fmt/format.h>
#include <algorithm>
#include <functional>
#include <map>
#include <quick_dra/base/meta.hpp>
#include <quick_dra/base/str.hpp>
#include <quick_dra/base/types.hpp>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace quick_dra {
	static constexpr auto kIndent = 2u;

	struct addition {
		std::vector<unsigned> refs;
	};

	struct varname {
		std::vector<std::string> path;

		static varname parse(std::string_view path) {
			if (path.starts_with('$')) path = path.substr(1);
			return varname{.path = split_s(path, '.'_sep)};
		}
	};

	struct compiletime_varname {
		std::string_view name;
		operator varname() const { return varname::parse(name); }
	};

	inline consteval compiletime_varname operator""_var(char const* data,
	                                                    size_t size) {
		return {.name{data, size}};
	}

	namespace var {
#define VAR_CONST static constexpr auto
#define VAR(N) VAR_CONST N = #N##_var
#define MEMBER_VAR(N, M) VAR_CONST M = #N "." #M##_var
#define VAR_BEGIN(N)                                              \
	struct N##_t {                                                \
	private:                                                      \
		VAR_CONST name_{#N##_var};                                \
                                                                  \
	public:                                                       \
		operator varname() const { return name_; }                \
		constexpr operator compiletime_varname() const noexcept { \
			return name_;                                         \
		}
#define VAR_END(N) \
	}              \
	;              \
	VAR_CONST N = N##_t{};
#define CONTRIBUTION_VAR(N) \
	VAR_BEGIN(N)            \
	MEMBER_VAR(N, payer);   \
	MEMBER_VAR(N, insured); \
	VAR_END(N)

		VAR(NN);
		VAR(DATE);
		VAR_BEGIN(serial)
		MEMBER_VAR(serial, NN);
		MEMBER_VAR(serial, DATE);
		VAR_END(serial);
		VAR(today);

		VAR(payer);
		VAR_BEGIN(insured)
		MEMBER_VAR(insured, document);
		MEMBER_VAR(insured, first);
		MEMBER_VAR(insured, last);
		VAR_END(insured);

		VAR(tax_id);
		VAR(social_id);
		VAR(name);
		VAR(last);
		VAR(first);
		VAR(document_kind);
		VAR(document);
		VAR(insurance_title);
		VAR(birthday);

		VAR(insured_count);
		VAR(accident_insurance_contribution);

		CONTRIBUTION_VAR(pension_insurance);
		CONTRIBUTION_VAR(disability_insurance);
		CONTRIBUTION_VAR(health_insurance);
		CONTRIBUTION_VAR(accident_insurance);
		CONTRIBUTION_VAR(guaranteed_employee_benefits_fund);
		VAR(health_baseline);
		VAR(health_contribution);
		VAR(insurance_total);
		VAR(tax_total);

		VAR_BEGIN(scale)
		MEMBER_VAR(scale, num);
		MEMBER_VAR(scale, den);
		VAR_END(scale);

		VAR_BEGIN(salary)
		MEMBER_VAR(salary, gross);
		MEMBER_VAR(salary, net);
		MEMBER_VAR(salary, payer_gross);
		VAR_END(salary);

#undef VAR
#undef MEMBER_VAR
#undef VAR_BEGIN
#undef VAR_END
#undef CONTRIBUTION_VAR
#undef VAR_CONST
	};  // namespace var

	using calculated_value = std::
	    variant<std::monostate, std::string, currency, percent, uint_value>;
	using compiled_value = expand_args_t<calculated_value, addition, varname>;

	template <typename ValueType>
	struct value_formatter {
		// GCOV_EXCL_START
		auto operator()(std::monostate) const { return "<null>"s; }
		// GCOV_EXCL_STOP
		auto operator()(std::string const& str) const {
			return fmt::format("'{}'", str);
		}
		auto operator()(currency const& value) const {
			return fmt::format("{:.2f} zł", value);
		}
		auto operator()(percent const& value) const {
			return fmt::format("{:.2f}%", value);
		}
		auto operator()(uint_value const& value) const {
			return fmt::format("{}", value);
		}
		auto operator()(addition const& sum) const {
			return fmt::format("({})", fmt::join(sum.refs, " + "));
		}
		auto operator()(varname const& var) const {
			return fmt::format("${}", fmt::join(var.path, "."));
		}
		auto operator()(ValueType const& value) const {
			return std::visit(*this, value);
		}
		auto operator()(std::vector<ValueType> const& values) const {
			std::string result{"["};
			bool first = true;
			for (auto const& value : values) {
				if (!first) result.append(", "sv);
				first = false;
				result.append(std::visit(*this, value));
			}
			result.push_back(']');
			return result;
		}

		auto operator()(maybe_list<ValueType> const& value) const {
			return std::visit(*this, value);
		}
	};

	template <typename ValueType>
	struct value_printer {
		using sink_type = std::function<void(std::string_view)>;

		explicit value_printer(sink_type sink = {}) : sink_{std::move(sink)} {
			if (!sink_) {
				sink_ = [](auto const& text) { fmt::print("{}", text); };
			}
		}

		explicit value_printer(FILE* file)
		    : value_printer{
		          [file](auto const& text) { fmt::print(file, "{}", text); }} {}

		void operator()(auto const& arg) const {
			sink_(value_formatter<ValueType>{}(arg));
		}

	private:
		sink_type sink_{};
	};

	template <typename ValueType>
	struct block {
		std::string id{};
		mapped_value<ValueType> fields{};
		void debug_print(int indent, bool standalone = true) const noexcept {
			bool first = standalone;
			if (!id.empty()) {
				fmt::print("-- {:{}}{} id: {}\n", "", indent, first ? '-' : ' ',
				           id);
				first = false;
			}

			fmt::print("-- {:{}}{} fields:\n", "", indent, first ? '-' : ' ');

			unsigned max_id = 0;
			for (auto const& [index, _] : fields) {
				max_id = std::max(index, max_id);
			}

			auto const width =
			    max_id ? static_cast<unsigned>(std::log10(max_id) + 1.0) : 1u;

			static const auto printer = value_printer<ValueType>{};

			for (auto const& [index, field] : fields) {
				fmt::print("-- {:{}}    {:{}}: ", "", indent, index, width);
				std::visit(printer, field);
				fmt::print("\n");
			}

			fmt::print("--\n");
		}
	};

	template <typename ValueType>
	struct section {
		std::string id{};
		bool repeatable{false};
		std::vector<block<ValueType>> blocks{};

		void debug_print() const noexcept {
			fmt::print("--     {}", id);
			if (repeatable) {
				fmt::print("[*]");
			}
			fmt::print(":\n");

			if (blocks.size() == 1) {
				blocks.front().debug_print(2 * kIndent, false);
			} else {
				for (auto const& block : blocks) {
					block.debug_print(3 * kIndent);
				}
			}
		}
	};

	template <typename ValueType>
	inline void debug_print(
	    std::vector<section<ValueType>> const& report) noexcept {
		for (auto const& section : report) {
			section.debug_print();
		}

		if (report.empty()) {
			fmt::print("\n");  // GCOV_EXCL_LINE
		}
	}

	using calculated_block = block<calculated_value>;
	using compiled_block = block<compiled_value>;

	using calculated_section = section<calculated_value>;
	using compiled_section = section<compiled_value>;

	calculated_block calculate(compiled_block const& self,
	                           struct global_object const& ctx,
	                           std::string_view log_name);

	calculated_section calculate(compiled_section const& self,
	                             global_object const& ctx);

	std::vector<calculated_section> calculate(
	    std::vector<compiled_section> const& report,
	    global_object const& ctx);

	namespace v1 {
		struct templates;
	};

	struct compiled_templates {
		std::map<std::string, std::vector<compiled_section>> reports;

		static compiled_templates compile(v1::templates const&);
		void debug_print() const noexcept;
	};
}  // namespace quick_dra
