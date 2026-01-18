// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <fmt/format.h>
#include <algorithm>
#include <map>
#include <quick_dra/base/meta.hpp>
#include <quick_dra/base/str.hpp>
#include <quick_dra/models/base_types.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace quick_dra {
	static constexpr auto INDENT = 2u;

	struct addition {
		std::vector<unsigned> refs;
	};

	struct varname {
		std::vector<std::string> path;

		static varname parse(std::string_view path) {
			if (path.starts_with('$')) path = path.substr(1);
			return varname{.path = split_s('.', path)};
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
#define CONST static constexpr auto
#define VAR(N) CONST N = #N##_var
#define MEMBER_VAR(N, M) CONST M = #N "." #M##_var
#define VAR_BEGIN(N)                                              \
	struct N##_t {                                                \
	private:                                                      \
		CONST name_{#N##_var};                                    \
                                                                  \
	public:                                                       \
		operator varname() const { return name_; }                \
		constexpr operator compiletime_varname() const noexcept { \
			return name_;                                         \
		}
#define VAR_END(N) \
	}              \
	;              \
	CONST N = N##_t{};
#define CONTRIBUTION_VAR(N)              \
	VAR_BEGIN(N) MEMBER_VAR(N, insurer); \
	MEMBER_VAR(N, insured);              \
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
		VAR_END(insured);
		VAR(insurer);

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
		VAR(amount_payable);

		VAR_BEGIN(scale)
		MEMBER_VAR(scale, num);
		MEMBER_VAR(scale, den);
		VAR_END(scale);

		VAR_BEGIN(remuneration)
		MEMBER_VAR(remuneration, gross);
		MEMBER_VAR(remuneration, net);
		MEMBER_VAR(remuneration, payer_gross);
		VAR_END(remuneration);

#undef VAR
#undef MEMBER_VAR
#undef VAR_BEGIN
#undef VAR_END
#undef CONTRIBUTION_VAR
#undef CONST
	};  // namespace var

	using calculated_value = std::
	    variant<std::monostate, std::string, currency, percent, uint_value>;
	using compiled_value = expand_args_t<calculated_value, addition, varname>;

	template <typename ValueType>
	struct value_printer {
		void operator()(std::monostate) const noexcept { fmt::print("<null>"); }
		void operator()(std::string const& str) const noexcept {
			fmt::print("'{}'", str);
		}
		void operator()(currency const& value) const noexcept {
			fmt::print("{:.2f} zł", value);
		}
		void operator()(percent const& value) const noexcept {
			fmt::print("{:.2f}%", value);
		}
		void operator()(uint_value const& value) const noexcept {
			fmt::print("{}", value);
		}
		void operator()(addition const& sum) const noexcept {
			fmt::print("({})", fmt::join(sum.refs, " + "));
		}
		void operator()(varname const& var) const noexcept {
			fmt::print("${}", fmt::join(var.path, "."));
		}
		void operator()(ValueType const& value) const noexcept {
			std::visit(*this, value);
		}
		void operator()(std::vector<ValueType> const& values) const noexcept {
			fmt::print("[");
			bool first = true;
			for (auto const& value : values) {
				if (!first) fmt::print(", ");
				first = false;
				std::visit(*this, value);
			}
			fmt::print("]");
		}
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
				blocks.front().debug_print(2 * INDENT, false);
			} else {
				for (auto const& block : blocks) {
					block.debug_print(3 * INDENT);
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
			fmt::print("\n");
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
