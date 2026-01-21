// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <quick_dra/base/str.hpp>
#include "_parser_types.hpp"

namespace quick_dra {
	namespace {
		compiled_value compile_field(std::string_view input);

		struct field_compiler {
			maybe_list<compiled_value> operator()(
			    std::string const& field) const noexcept {
				return compile_field(field);
			}
			maybe_list<compiled_value> operator()(
			    std::vector<std::string> const& fields) const noexcept {
				std::vector<compiled_value> result{};
				result.reserve(fields.size());
				std::transform(fields.begin(), fields.end(),
				               std::back_inserter(result), compile_field);
				return result;
			}
		};

		struct section_stats {
			size_t block_count{0};
			bool repeatable{false};
		};

		compiled_value compile_sum(std::string_view input) {
			auto const parts = split_sv(input.substr(2), ','_sep);
			std::vector<unsigned> refs{};
			refs.reserve(parts.size());

			for (auto const& part : parts) {
				auto const trimmed = strip_sv(part);
				unsigned ref{};

				auto const begin = trimmed.data();
				auto const end = begin + trimmed.size();
				auto const [ptr, ec] = std::from_chars(begin, end, ref);
				if (ptr != end || ec != std::errc{}) {
					fmt::print(stderr, "error while parsing `{}'\n", input);
					return fmt::format("unparsable: {}", input);
				}

				refs.push_back(ref);
			}
			return addition{.refs = std::move(refs)};
		}

		compiled_value compile_currency(std::string_view input) {
			auto const view =
			    strip_sv(input.substr(0, input.size() - "zł"sv.size()));
			currency value{};
			if (!currency::parse(view, value)) {
				fmt::print(stderr, "error while parsing `{}'\n", input);
				return fmt::format("unparsable: {}", input);
			}

			return value;
		}

		compiled_value compile_field(std::string_view input) {
			if (input.starts_with('$')) {
				if (input.starts_with("$+"sv)) {
					return compile_sum(input);
				}

				return varname::parse(input);
			}

			if (input.ends_with("zł")) {
				return compile_currency(input);
			}

			return std::string{input.data(), input.size()};
		}

		compiled_block compile_block(report_section const& input) {
			compiled_block result{};

			if (input.block) {
				result.id = *input.block;
			}

			for (auto const& [index, field] : input.fields) {
				result.fields[index] = std::visit(field_compiler{}, field);
			}

			return result;
		}

		std::vector<compiled_section> compile_report(
		    std::vector<report_section> const& input) {
			std::map<std::string, section_stats> stats{};
			for (auto const& section : input) {
				auto it = stats.lower_bound(section.id);
				if (it == stats.end() || it->first != section.id) {
					it = stats.insert(it, {section.id, section_stats{}});
				}

				auto& stat = it->second;

				++stat.block_count;
				stat.repeatable |= section.repeatable.value_or(false);
			}
			std::vector<compiled_section> result{};
			result.reserve(stats.size());
			for (auto const& section : input) {
				if (section.fields.empty() && !section.block) continue;

				auto it = std::find_if(
				    result.begin(), result.end(),
				    [&id = section.id](compiled_section const& out) {
					    return out.id == id;
				    });

				if (it == result.end()) {
					size_t block_count = 0;
					bool repeatable = false;
					auto stat_it = stats.find(section.id);

					if (stat_it != stats.end()) {
						block_count = stat_it->second.block_count;
						repeatable = stat_it->second.repeatable;
					}

					result.push_back({section.id, repeatable});

					if (block_count) {
						result.back().blocks.reserve(block_count);
					}

					it = std::prev(result.end());
				}

				auto& output = *it;
				output.blocks.push_back(compile_block(section));
			}
			return result;
		}

		struct data_calculator {
			std::string_view id;
			global_object const& ctx;
			mapped_value<compiled_value> fields;
			std::map<unsigned, std::set<unsigned>> crossrefs{};

			static mapped_value<calculated_value> calculate(
			    std::string_view id,
			    mapped_value<compiled_value> const& fields,
			    global_object const& ctx) {
				data_calculator src{id, ctx, fields};
				return src.calculate();
			}

		private:
			static constexpr auto invalid_index =
			    std::numeric_limits<size_t>::max();

			template <typename From, typename To>
			struct value_extractor {
				To operator()(auto const& value) const noexcept {
					return value;
				}
				To operator()(addition const&) const noexcept { return {}; }
				To operator()(varname const&) const noexcept { return {}; }

				maybe_list<To> operator()(From const& field) const noexcept {
					return std::visit(*this, field);
				}

				maybe_list<To> operator()(
				    std::vector<From> const& fields) const noexcept {
					std::vector<To> result{};
					result.reserve(fields.size());
					for (auto const& field : fields) {
						result.push_back(std::visit(*this, field));
					}
					return result;
				}
			};

			template <typename To, typename From>
			maybe_list<To> extract(maybe_list<From> const& val) {
				return std::visit(value_extractor<From, To>{}, val);
			}

			template <typename To, typename From>
			To extract(From const& val) {
				return std::visit(value_extractor<From, To>{}, val);
			}

			inline std::string label(unsigned key,
			                         size_t index = invalid_index) {
				return fmt::format(
				    fmt::runtime(index == invalid_index ? "{} p{}"
				                                        : "{} p{}.{}"),
				    id, key, index);
			}

			void fill_var(unsigned key,
			              size_t index,
			              compiled_value& tgt,
			              varname const& var) {
				auto const ptr = ctx.peek(var);
				if (!ptr) {
					fmt::print(stderr, "{}: error: cannot find `${}'\n",
					           label(key, index), join(var.path, '.'_sep));
					return;
				}

				auto const& data = *ptr;

				if (!data.value) {
					fmt::print(stderr,
					           "{}: error: reference `${}' contains no "
					           "value\n",
					           label(key, index), join(var.path, '.'_sep));
					return;
				}

				auto const& src = *data.value;
				if (index == invalid_index) {
					fields.find(key)->second = extract<compiled_value>(src);
					return;
				}

				if (std::holds_alternative<calculated_value>(src)) {
					tgt = extract<compiled_value>(
					    std::get<calculated_value>(src));
					return;
				}

				fmt::print(stderr,
				           "{}: error: cannot assign a list to a "
				           "list item when checking `${}'\n",
				           label(key, index), join(var.path, '.'_sep));
			}

			void precalc(unsigned key, size_t index, compiled_value& tgt) {
				if (std::holds_alternative<varname>(tgt)) {
					fill_var(key, index, tgt, std::get<varname>(tgt));
					return;
				}

				if (std::holds_alternative<addition>(tgt)) {
					auto const& refs = std::get<addition>(tgt).refs;

					if (index != invalid_index) {
						fmt::print(stderr,
						           "{}: error: field addition inside a "
						           "sub-field ({})\n",
						           label(key, index), fmt::join(refs, " + "));
						return;
					}

					crossrefs[key] =
					    std::set<unsigned>{refs.begin(), refs.end()};
				}
			}

			void print_crossrefs() {
				for (auto const& pair : crossrefs) {
					fmt::print("{} {}\n", id, pair);
				}
			}

			std::set<unsigned> get_crossrefs_layer() {
				std::set<unsigned> result{};

				for (auto& [key, refs] : crossrefs) {
					auto copy = refs;
					for (auto ref : copy) {
						auto it = crossrefs.find(ref);
						if (it == crossrefs.end()) {
							refs.erase(ref);
						}
					}

					if (refs.empty()) {
						result.insert(key);
					}
				}

				for (auto key : result) {
					crossrefs.erase(key);
				}

				// fmt::print("\n");
				// print_crossrefs();
				// fmt::print(" -- {}\n", result);

				return result;
			}

			void calculate_sum(unsigned key) {
				auto& tgt = std::get<compiled_value>(fields.find(key)->second);
				auto& refs = std::get<addition>(tgt).refs;

				currency result{};

				for (auto ref : refs) {
					auto it = fields.find(ref);
					if (it == fields.end()) {
						fmt::print(stderr, "{}: error: cannot find p{}\n",
						           label(key), ref);
						return;
					}

					auto src = std::get_if<compiled_value>(&it->second);
					if (!src) {
						fmt::print(stderr, "{}: error: p{} is not a scalar\n",
						           label(key), ref);
						return;
					}

					auto val = std::get_if<currency>(src);
					if (!val) {
						fmt::print(stderr, "{}: error: p{} is not a number\n",
						           label(key), ref);
						return;
					}

					result = result + *val;
				}

				tgt = result;
			}

			void remove_crossrefs() {
				// print_crossrefs();
				auto layer = get_crossrefs_layer();
				while (!layer.empty()) {
					for (auto key : layer) {
						calculate_sum(key);
					}
					layer = get_crossrefs_layer();
				}
			}

			mapped_value<calculated_value> calculate() {
				for (auto& [key, field] : fields) {
					if (std::holds_alternative<compiled_value>(field)) {
						precalc(key, invalid_index,
						        std::get<compiled_value>(field));
					} else {
						size_t index = 0;
						for (auto& child :
						     std::get<std::vector<compiled_value>>(field)) {
							precalc(key, index, child);
							++index;
						}
					}
				}

				remove_crossrefs();

				mapped_value<calculated_value> result{};
				for (auto const& [key, field] : fields) {
					result[key] = extract<calculated_value>(field);
				}
				return result;
			}
		};
	}  // namespace

	calculated_block calculate(compiled_block const& self,
	                           global_object const& ctx,
	                           std::string_view log_name) {
		std::string extended_name;
		if (!self.id.empty()) {
			extended_name = fmt::format("{}.{}", log_name, self.id);
			log_name = extended_name;
		}
		return calculated_block{
		    .id = self.id,
		    .fields = data_calculator::calculate(log_name, self.fields, ctx)};
	}

	calculated_section calculate(compiled_section const& self,
	                             global_object const& ctx) {
		calculated_section result{.id = self.id, .repeatable = self.repeatable};
		result.blocks.reserve(self.blocks.size());
		for (auto const& block : self.blocks) {
			result.blocks.push_back(calculate(block, ctx, self.id));
		}
		return result;
	}

	std::vector<calculated_section> calculate(
	    std::vector<compiled_section> const& report,
	    global_object const& ctx) {
		std::vector<calculated_section> result{};
		result.reserve(report.size());
		for (auto const& section : report) {
			result.push_back(calculate(section, ctx));
		}
		return result;
	}

	compiled_templates compiled_templates::compile(templates const& input) {
		compiled_templates result{};
		for (auto const& [key, report] : input.reports) {
			result.reports[key] = compile_report(report);
		}
		return result;
	}
}  // namespace quick_dra
