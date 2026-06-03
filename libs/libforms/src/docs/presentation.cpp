// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <algorithm>
#include <array>
#include <map>
#include <quick_dra/docs/presentation.hpp>
#include <string>
#include <utility>
#include <vector>
#include <yaml/parser.hpp>

namespace quick_dra {
	using yaml::enum_tag;

	auto const& enums(alignement) noexcept {
		static constexpr auto names = std::array{
		    enum_tag{.id = "left"sv, .value = alignement::left},
		    enum_tag{.id = "<"sv, .value = alignement::left},
		    enum_tag{.id = "right"sv, .value = alignement::right},
		    enum_tag{.id = ">"sv, .value = alignement::right},
		};
		return names;
	}
}  // namespace quick_dra

namespace quick_dra::report_format {
	using yaml::read_key;
	using yaml::read_value;

	struct doc_labels {
		std::string section_label{};
		std::map<unsigned, std::string> fields{};
	};

	struct yaml_shape {
		std::map<std::string, std::vector<ref>> titles{};
		std::map<std::string, std::map<std::string, hint>> hints{};
		std::map<std::string, std::map<std::string, doc_labels>> labels{};
	};

	struct value_printer {
		// GCOV_EXCL_START
		std::string operator()(std::monostate) const noexcept { return {}; }
		// GCOV_EXCL_STOP
		std::string operator()(std::string const& str) const noexcept { return str; }
		std::string operator()(currency const& value) const noexcept { return fmt::format("{:.2f} zł", value); }
		std::string operator()(percent const& value) const noexcept { return fmt::format("{:.2f}%", value); }
		std::string operator()(uint_value const& value) const noexcept { return fmt::format("{}", value); }
		std::string operator()(year_month const& var) const {
			return fmt::format("{:02}-{:04}", static_cast<unsigned>(var.month()), static_cast<int>(var.year()));
		}
		std::string operator()(year_month_day const& var) const {
			return fmt::format("{:02}-{:02}-{:04}", static_cast<unsigned>(var.day()),
			                   static_cast<unsigned>(var.month()), static_cast<int>(var.year()));
		}
	};

	bool read_value(yaml::ref_ctx const& ref, hint& ctx) {
		if (!ref.ref().is_map()) {
			alignement a{};
			if (read_value(ref, a)) {
				ctx.alignment_ = a;
				return true;
			}
			return false;
		}
		if (!read_key(ref, "sep", ctx.sep, true)) return ref.error("while reading `sep'");
		if (!read_key(ref, "align", ctx.alignment_)) return ref.error("while reading `align'");
		if (!read_key(ref, "ignore", ctx.ignore_)) return ref.error("while reading `ignore'");
		return true;
	}

	bool read_value(yaml::ref_ctx const& ref, report_format::ref& ctx) {
		std::string str;
		if (!read_value(ref, str)) return false;
		if (str.starts_with('=')) {
			ctx.section = str.substr(1);
			ctx.field = 0;
			return true;
		}
		auto const split = split_sv(str, '.'_sep);
		if (split.size() < 2 || split.size() > 3) {
			return ref.error(fmt::format("expected SECTION[.BLOCK].FIELD, got `{}`", str));
		}

		unsigned field{};
		if (!yaml::convert_string(ref, split[split.size() - 1], field)) {
			return false;
		}

		ctx.section = split[0];
		ctx.field = field;
		if (split.size() == 2)
			ctx.block.clear();
		else
			ctx.block = split[1];
		return true;
	}

	bool read_value(yaml::ref_ctx const& ref, doc_labels& ctx) {
		if (!read_key(ref, "label", ctx.section_label, true)) return ref.error("while reading `label'");
		if (!read_key(ref, "fields", ctx.fields, true)) return ref.error("while reading `fields'");
		return true;
	}

	bool read_value(yaml::ref_ctx const& ref, yaml_shape& ctx) {
		if (!read_key(ref, "titles", ctx.titles, true)) return ref.error("while reading `titles'");
		if (!read_key(ref, "format hints", ctx.hints, true)) return ref.error("while reading `format hints'");
		if (!read_key(ref, "labels", ctx.labels, true)) return ref.error("while reading `labels'");
		return true;
	}

#if 0
	bool read_value(yaml::ref_ctx const& ref, formatting& ctx) {
		if (!ref.ref().is_map()) {
			return ref.error("expecting a map"sv);
		}

		for (auto const& child : ref.ref()) {
			auto const child_var = ref.from(child);

			if (!child.has_key()) {
				// GCOV_EXCL_START
				return child_var.error("expecting a map member");
			}  // GCOV_EXCL_STOP

			std::string key;
			if (!convert_string(child_var, child.key(), key)) return false;

			if (key == "_title"sv) {
				if (!read_value(child_var, ctx.title)) return false;
				continue;
			}

			auto& sub = ctx.hints[std::move(key)];
			if (!read_value(child_var, sub)) return false;
		}
		return true;
	}
#endif

	std::string ref::title_chunk_from(calculated_block const& form_block) const {
		auto const& fields = form_block.fields;
		auto fld_it = fields.find(field);
		if (fld_it != fields.end()) {
			auto value = std::get_if<calculated_value>(&fld_it->second);
			if (value) return std::visit(value_printer{}, *value);
		}
		return "?"s;
	}

	std::string ref::get_chunk(std::vector<calculated_section> const& form) const {
		if (!field) {
			return section;
		}

		auto sec_it =
		    std::find_if(form.begin(), form.end(), [self = this](auto const& sec) { return self->section == sec.id; });
		if (sec_it != form.end()) {
			if (block.empty()) {
				if (sec_it->blocks.size() == 1) {
					return title_chunk_from(sec_it->blocks.front());
				}
			} else {  // GCOV_EXCL_LINE[WIN32]
				auto block_it = std::find_if(sec_it->blocks.begin(), sec_it->blocks.end(),
				                             [self = this](auto const& blk) { return self->block == blk.id; });
				if (block_it != sec_it->blocks.end()) {
					return title_chunk_from(*block_it);
				}
			}  // GCOV_EXCL_LINE[WIN32]
		}

		return "?"s;
	}

	hint const& formatting::get_hint(std::string const& key) const noexcept {
		auto it = hints.find(key);
		if (it == hints.end()) {
			static auto const def_hint = hint{};
			return def_hint;
		}
		return it->second;
	}

	std::string formatting::get_title(std::vector<calculated_section> const& form) const {
		std::vector<std::string> chunks{};
		chunks.reserve(title.size());
		std::transform(title.begin(), title.end(), std::back_inserter(chunks),
		               [&form](auto const& ref) { return ref.get_chunk(form); });

		size_t length{};
		for (auto const& chunk : chunks) {
			length += chunk.size();
		}

		std::string result{};
		result.reserve(length);
		for (auto const& chunk : chunks) {
			result += chunk;
		}

		return result;
	}

	void formatting::visit_block(calculated_block const& block,
	                             bool inherited_ignore,
	                             std::string const& section,
	                             formatted_report& report) const {
		for (auto const& [pos, field] : block.fields) {
			auto new_key = fmt::format("{}.{}", section, pos);
			auto const& hint = get_hint(new_key);
			if (hint.ignore(inherited_ignore)) {
				continue;
			}
			auto label_it = labels.find(new_key);

			auto value = std::get_if<calculated_value>(&field);
			if (value) {
				auto data = std::visit(value_printer{}, *value);
				report.add(section,
				           {
				               .number = pos,
				               .formatted = std::move(data),
				               .label = label_it != labels.end() ? label_it->second : ""s,
				               .alignement = hint.alignment_for(*value),
				           },
				           labels);
			} else {
				auto const& list = std::get<std::vector<calculated_value>>(field);
				std::vector<std::string> parts{};
				parts.reserve(list.size());
				std::transform(list.begin(), list.end(), std::back_inserter(parts),
				               [](auto const& item) { return std::visit(value_printer{}, item); });
				auto size = hint.sep.size() * (parts.empty() ? 0 : (parts.size() - 1));
				for (auto const& item : parts) {
					size += item.size();
				}
				std::string data{};
				data.reserve(size);
				for (auto const& item : parts) {
					if (!data.empty()) data += hint.sep;
					data += item;
				}
				auto const alignment = hint.alignment_.value_or(alignement::left);
				report.add(section,
				           {
				               .number = pos,
				               .formatted = std::move(data),
				               .label = label_it != labels.end() ? label_it->second : ""s,
				               .alignement = alignment,
				           },
				           labels);
			}
		}
	}

	void formatting::visit_section(calculated_section const& section, formatted_report& report) const {
		auto const section_ignore = get_hint(section.id).ignore(false);
		if (section.blocks.size() == 1) {
			return visit_block(section.blocks.front(), section_ignore, section.id, report);
		}

		for (auto const& block : section.blocks) {
			auto section_id = fmt::format("{}.{}", section.id, block.id);
			auto const block_ignore = get_hint(section_id).ignore(section_ignore);
			visit_block(block, block_ignore, section_id, report);
		}
	}

	formatted_report formatting::format(std::string const& key, std::vector<calculated_section> const& form) const {
		formatted_report report{};

		report.title = get_title(form);
		report.title = report.title.empty() ? key : fmt::format("{} ({})", key, report.title);

		for (auto const& section : form) {
			visit_section(section, report);
		}
		return report;
	}  // GCOV_EXCL_LINE[GCC]

	std::map<std::string, formatting> formatting::parse(std::string const& text, std::string const& path) {
		auto split = yaml::parser::parse_yaml_text<yaml_shape>(text, path);
		if (!split) {
			return {};
		}

		std::map<std::string, formatting> result{};

		for (auto& [key, item] : split->titles) {
			auto it = result.lower_bound(key);
			if (it == result.end() || it->first != key) {
				it = result.insert(it, {key, {}});
			}

			it->second.title = std::move(item);
		}

		for (auto& [key, item] : split->hints) {
			auto it = result.lower_bound(key);
			if (it == result.end() || it->first != key) {
				it = result.insert(it, {key, {}});
			}

			it->second.hints = std::move(item);
		}

		for (auto& [key, sections] : split->labels) {
			auto it = result.lower_bound(key);
			if (it == result.end() || it->first != key) {
				it = result.insert(it, {key, {}});
			}
			auto& labels = it->second.labels;

			for (auto const& [section_key, item] : sections) {
				if (!item.section_label.empty()) {
					labels[section_key] = std::move(item.section_label);
				}
				for (auto& [field, dscr] : item.fields) {
					if (dscr.empty()) continue;
					labels[fmt::format("{}.{}", section_key, field)] = std::move(dscr);
				}
			}
		}

		return result;
	}

	formatted_report formatting::format_report(std::map<std::string, formatting> const& formats,
	                                           std::string const& key,
	                                           std::vector<calculated_section> const& form) {
		formatting empty{};
		auto it = formats.find(key);
		auto const& formatter = it != formats.end() ? it->second : empty;

		return formatter.format(key, form);
	}
}  // namespace quick_dra::report_format
