// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <fmt/std.h>
#include <cmath>
#include <fstream>
#include <quick_dra/base/str.hpp>
#include <quick_dra/models/project_reader.hpp>

namespace quick_dra::v1 {
	namespace {
		static constexpr auto app_name = "Quick-DRA"sv;
	}

	std::optional<config> config::parse_yaml(
	    std::filesystem::path const& path) {
		return parser::parse_yaml_file<config>(path, app_name);
	}

	bool config::postprocess() { return version == kVersion; }

	template <typename Named>
	bool parse_and_validate_name(Named& named) noexcept {
		auto const name = split_sv(named.last_name, ", "_sep, 1);
		if (name.size() != 2) return false;
		named.first_name = strip_sv(name[1]);
		named.last_name = strip_sv(name[0]);
		return !(named.last_name.empty() || named.first_name.empty());
	}

	bool payer_t::postprocess() {
		kind.clear();
		document.clear();

		if (id_card) {
			if (passport) {
				return false;
			}
			kind = "1"sv;
			document = std::move(*id_card);
		}

		if (passport) {
			kind = "2"sv;
			document = std::move(*passport);
		}

		return parse_and_validate_name(*this) &&
		       !(social_id.empty() || tax_id.empty() || kind.empty() ||
		         document.empty());
	}

	bool insured_t::postprocess() {
		if (!parse_and_validate_name(*this)) return false;

		kind.clear();
		document.clear();

		if (social_id) {
			if (id_card || passport) {
				return false;
			}
			kind = "P"sv;
			document = std::move(*social_id);
		}

		if (id_card) {
			if (social_id || passport) {
				return false;
			}
			kind = "1"sv;
			document = std::move(*id_card);
		}

		if (passport) {
			if (social_id || id_card) {
				return false;
			}
			kind = "2"sv;
			document = std::move(*passport);
		}

		if (kind.empty() || document.empty()) return false;
		return true;
	}

	std::optional<templates> templates::parse_yaml(
	    std::filesystem::path const& path) {
		return parser::parse_yaml_file<templates>(path, app_name);
	}

	bool templates::validate() noexcept {
		for (auto& [kedu, report] : reports) {
			if (kedu.empty()) return false;
			for (auto& section : report) {
				if (!section.validate()) return false;
			}
		}
		return true;
	}

	bool report_section::validate() noexcept {
		if (id.empty()) return false;

		struct validator {
			bool operator()(std::string const& str) const noexcept {
				return !str.empty();
			}
			bool operator()(
			    std::vector<std::string> const& vec) const noexcept {
				if (vec.empty()) return false;
				for (auto const& str : vec) {
					if (str.empty()) return false;
				}
				return true;
			}
		} value{};

		for (auto const& [_, field] : fields) {
			if (!std::visit(value, field)) return false;
		}
		return true;
	}

	std::optional<tax_config> tax_config::parse_yaml(
	    std::filesystem::path const& path) {
		return parser::parse_yaml_file<tax_config>(path, app_name);
	}

	std::optional<tax_config> tax_config::parse_from_text(
	    std::string const& text,
	    std::string const& path) {
		return parser::parse_yaml_text<tax_config>(text, path);
	}

	bool tax_config::postprocess() { return version == kVersion; }

	void tax_config::merge(tax_config&& newer) {
		scale.merge(std::move(newer.scale));
		minimal_pay.merge(std::move(newer.minimal_pay));
		costs_of_obtaining.merge(std::move(newer.costs_of_obtaining));
		contributions.merge(std::move(newer.contributions));
	}
}  // namespace quick_dra::v1

namespace quick_dra::v1::partial {
	load_status config::load(std::filesystem::path const& path) {
		std::error_code ec{};
		if (!std::filesystem::exists(path, ec) || ec) {
			return load_status::file_not_found;
		}

		auto result = load_status::errors_encountered;

		auto object = parser::parse_yaml_file<config>(
		    path, [&]() { result = load_status::file_not_readable; });
		if (!object) {
			return result;
		}

		*this = std::move(*object);
		auto const [needed, present] = count_loaded();
		return needed < present ? needed == 0 ? load_status::empty
		                                      : load_status::partially_loaded
		                        : load_status::fully_loaded;
	}

	config config::load_partial(std::filesystem::path const& path,
	                            bool writeable) {
		partial::config cfg{};
		auto const load = cfg.load(path);
		switch (load) {
			case load_status::file_not_found:
				if (writeable) {
					fmt::print(
					    stderr,
					    "Quick-DRA: file {} will be created as needed.\n",
					    path);
				}
				break;
			case load_status::file_not_readable:
				fmt::print(stderr, "Quick-DRA: error: could not read {}\n",
				           path);
				std::exit(1);
			case load_status::errors_encountered:
				fmt::print(stderr, "Quick-DRA: error: {} needs to be updated\n",
				           path);
				std::exit(1);
			default:
				break;
		}
		return cfg;
	}

	bool config::store(std::filesystem::path const& path) {
		prepare_for_write();
		ryml::Tree tree{};
		auto ref = tree.rootref();
		yaml::write_value(ref, *this);

		ryml::csubstr output = ryml::emit_yaml(
		    tree, tree.root_id(), ryml::substr{}, /*error_on_excess*/ false);

		std::vector<char> buf(output.len);
		output = ryml::emit_yaml(tree, tree.root_id(), ryml::to_substr(buf),
		                         /*error_on_excess*/ true);

		std::ofstream out{path, std::ios::out | std::ios::binary};
		if (!out) {
			return false;
		}

		out.write(output.data(), static_cast<std::streamsize>(output.size()));
		return true;
	}

	bool config::postprocess() {
		if (version && *version != kVersion) version = std::nullopt;
		if (!insured) insured.emplace();
		return true;
	}

	void config::preprocess() {
		if (!version) version = static_cast<unsigned short>(kVersion);
		if (!insured) insured.emplace();
	}

	template <typename Named>
	void parse_name(Named& named) noexcept {
		if (!named.last_name) return;

		auto const name = split_s(*named.last_name, ", "_sep, 1);
		if (name.size() == 2) {
			named.first_name.emplace(strip_sv(name[1]));
			named.last_name.emplace(strip_sv(name[0]));
			if (!(named.last_name->empty() || named.first_name->empty())) {
				return;
			}
		}

		named.first_name = std::nullopt;
		named.last_name = std::nullopt;
	}

	template <typename Named>
	void preprocess_name(Named& named) noexcept {
		if (!named.last_name || !named.first_name) {
			named.last_name = std::nullopt;
			return;
		}

		named.last_name =
		    fmt::format("{}, {}", *named.last_name, *named.first_name);
		named.first_name = std::nullopt;
	}

	void payer_t::postprocess_document_kind() noexcept {
		kind = std::nullopt;
		document = std::nullopt;

		if (id_card) {
			kind = "1"sv;
			document = std::move(*id_card);
		} else if (passport) {
			kind = "2"sv;
			document = std::move(*passport);
		}
	}

	bool payer_t::postprocess() {
		postprocess_document_kind();

		parse_name(*this);

#define NULLIFY(STR)           \
	if (STR && STR->empty()) { \
		STR.reset();           \
	}

		NULLIFY(social_id);
		NULLIFY(tax_id);
		NULLIFY(kind);
		NULLIFY(document);
		return true;
	}

	void payer_t::preprocess() {
		preprocess_name(*this);
		preprocess_document_kind();
	}

	void payer_t::preprocess_document_kind() noexcept {
		auto kind_ = kind.value_or(""s);
		auto document_ = document.value_or(""s);

		id_card = std::nullopt;
		passport = std::nullopt;
		kind = std::nullopt;
		document = std::nullopt;

		if (kind_ == "1"sv) {
			id_card = std::move(document_);
		} else if (kind_ == "2"sv) {
			passport = std::move(document_);
		}
	}

	void insured_t::postprocess_document_kind() noexcept {
		kind = std::nullopt;
		document = std::nullopt;

		if (id_card) {
			kind = "1"sv;
			document = std::move(*id_card);
		} else if (passport) {
			kind = "2"sv;
			document = std::move(*passport);
		} else if (social_id) {
			kind = "P"sv;
			document = std::move(*social_id);
		}
	}

	bool insured_t::postprocess() {
		postprocess_document_kind();
		parse_name(*this);

		NULLIFY(id_card);
		NULLIFY(passport);
		NULLIFY(social_id);
		NULLIFY(kind);
		NULLIFY(document);

		return true;
	}

	void insured_t::preprocess() {
		preprocess_name(*this);
		preprocess_document_kind();
	}

	void insured_t::preprocess_document_kind() noexcept {
		auto kind_ = kind.value_or(""s);
		auto document_ = document.value_or(""s);

		id_card = std::nullopt;
		passport = std::nullopt;
		social_id = std::nullopt;
		kind = std::nullopt;
		document = std::nullopt;

		if (kind_ == "1"sv) {
			id_card = std::move(document_);
		} else if (kind_ == "2"sv) {
			passport = std::move(document_);
		} else if (kind_ == "P"sv) {
			social_id = std::move(document_);
		}
	}
}  // namespace quick_dra::v1::partial
