// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fstream>
#include <quick_dra/docs/xml.hpp>
#include <quick_dra/docs/xml_builder.hpp>

namespace quick_dra {
	namespace {
		struct xml_printer {
			std::string operator()(std::monostate) const noexcept { return {}; }
			std::string operator()(std::string const& str) const noexcept {
				return str;
			}
			std::string operator()(currency const& value) const noexcept {
				return fmt::format("{:.2f}", value);
			}
			std::string operator()(percent const& value) const noexcept {
				return fmt::format("{:.2f}", value);
			}
			std::string operator()(uint_value const& value) const noexcept {
				return fmt::format("{}", value);
			}
		};

		void append_field(xml& parent,
		                  unsigned key,
		                  calculated_value const& value) {
			if (std::holds_alternative<std::monostate>(value)) {
				return;
			}

			parent.with(E(fmt::format("p{}", key))
			                .with(std::visit(xml_printer{}, value)));
		}

		void append_block(xml& parent,
		                  mapped_value<calculated_value> const& fields) {
			for (auto const& [key, field] : fields) {
				auto value = std::get_if<calculated_value>(&field);
				if (value) {
					append_field(parent, key, *value);
					continue;
				}

				auto compound = E(fmt::format("p{}", key));
				unsigned index = 0;
				for (auto const& item :
				     std::get<std::vector<calculated_value>>(field)) {
					append_field(compound, ++index, item);
				}

				parent.with(std::move(compound));
			}
		}

		xml from_section(calculated_section const& section) {
			auto result = E(section.id);
			if (section.repeatable) {
				result.attributes["id_bloku"] = "1";
			}

			if (section.blocks.size() == 1 &&
			    section.blocks.front().id.empty()) {
				append_block(result, section.blocks.front().fields);
			} else {
				for (auto const& block : section.blocks) {
					if (block.id.empty()) {
						append_block(result, block.fields);
					} else {
						append_block(result.with(E(block.id)).children().back(),
						             block.fields);
					}
				}
			}

			return result;
		}

		xml map_sections(xml&& root,
		                 std::vector<calculated_section> const& sections) {
			for (auto const& section : sections) {
				root.with(from_section(section));
			}
			return root;
		}

		xml naglowek_kedu(std::string_view program_name,
		                  std::string_view version) {
			return E("naglowek.KEDU"sv)
			    .with(E("program"sv)
			              .with(E("producent"sv).with("midnightBITS"sv))
			              .with(E("symbol"sv).with(program_name))
			              .with(E("wersja"sv).with(version)));
		}
	};  // namespace

	xml build_kedu_doc(std::string_view program_name,
	                   std::string_view version) {
		return E("KEDU"sv,
		         {
		             {"xmlns"s, "http://www.zus.pl/2024/KEDU_5_6"s},
		             {"wersja_schematu"s, "1"s},
		         })
		    .with(naglowek_kedu(program_name, version));
	}

	void attach_document(xml& root,
	                     verbose level,
	                     form const& form,
	                     std::vector<compiled_section> const& tmplt,
	                     unsigned doc_id) {
		auto const sections = form.fill(level, tmplt);
		root.with(map_sections(E(fmt::format("ZUS{}", form.key),
		                         {{"id_dokumentu", fmt::to_string(doc_id)}}),
		                       sections));
	}

	void store_xml(xml const& tree, std::string const& filename) {
		fmt::print("-- output: {}\n", filename);
		std::ofstream{filename} << tree;
	}
};  // namespace quick_dra
