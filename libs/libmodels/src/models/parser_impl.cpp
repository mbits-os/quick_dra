// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <cmath>
#include <quick_dra/base/str.hpp>
#include "_parser_types.hpp"

namespace quick_dra::v1 {
	bool read_value(ref_ctx const& ref, bool& ctx) {
		if (!ref.ref().has_val()) {
			ctx = false;
			return false;
		}

		auto const value = ref.val();
		if (value.empty()) {
			ctx = false;
			return true;
		}

		auto key = std::string{value.data(), value.size()};
		for (auto& c : key) {
			c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
		}

		if (key == "true"sv) {
			ctx = true;
		} else if (key == "false"sv) {
			ctx = false;
		} else {
			return ref.error("expecting a true or false");
		}

		return true;
	}

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
		static constexpr auto expecting_NUM_DEN =
		    "expecting N/M, e.g. 1/1 or 4/5"sv;

		if (!ref.ref().has_val()) {
			ctx = {};
			return false;
		}

		auto const value = ref.val();
		if (value.empty()) {
			ctx = {};
			return false;
		}

		auto const split = split_sv('/', view(value));
		if (split.size() < 2) {
			ctx = {};
			return ref.error(expecting_NUM_DEN);
		}

		unsigned num{};
		unsigned den{};

		if (!convert_string(ref, split[0], num) ||
		    !convert_string(ref, split[1], den)) {
			return ref.error(expecting_NUM_DEN);
		}

		ctx = {.num = num, .den = den};
		return true;
	}

	bool read_value(ref_ctx const& ref, insurance_title& ctx) {
		return read_value(ref, ctx.code);
	}

	bool read_value(ref_ctx const& ref, std::string& ctx) {
		if (!ref.ref().has_val()) {
			ctx.clear();
			return false;
		}

		auto const value = ref.val();
		if (value.empty()) {
			ctx = {};
			return false;
		}

		ctx = {value.data(), value.size()};
		return true;
	}

	bool convert_string(ref_ctx const& ref,
	                    c4::csubstr const& value,
	                    std::chrono::year_month& ctx) {
		static constexpr auto expecting_YYYY_MM =
		    "expecting YYYY/MM or YYYY-MM"sv;
		auto const val = view(value);
		auto split = split_sv('/', val, 1);

		if (split.size() < 2) {
			split = split_sv('-', val, 1);
		}

		if (split.size() < 2) {
			return ref.error(expecting_YYYY_MM);
		}

		int year{};
		unsigned month{};

		if (!convert_string(ref, split[0], year) ||
		    !convert_string(ref, split[1], month)) {
			return ref.error(expecting_YYYY_MM);
		}

		ctx = std::chrono::year{year} / static_cast<int>(month);

		if (!ctx.ok()) {
			return ref.error(expecting_YYYY_MM);
		}

		return true;
	}

	namespace {
		template <typename T>
		    requires requires(T& obj) {
			    { obj.validate() } -> std::same_as<bool>;
		    }
		bool validate(T& obj) {
			return obj.validate();
		}
		bool validate(auto const&) { return true; }

		template <typename FileObj, typename Callback>
		std::optional<FileObj> parse_yaml_file_base(Callback&& cb) try {
			std::optional<FileObj> result{};

			c4::yml::Callbacks callbacks;
			callbacks.m_error = c4_error_handler;
			c4::yml::set_callbacks(callbacks);

			YAML parser{};

			auto maybe_tree = cb(parser);
			if (!maybe_tree) {
				return result;
			}
			auto& tree = *maybe_tree;

			result.emplace();

			parse_succeeded = true;
			auto root = tree.rootref();

			if (!read_value(parser.context().from(root), *result) ||
			    !validate(*result)) {
				result.reset();
			}

			return result;
		} catch (c4_error_exception const&) {
			return std::nullopt;
		}

		template <typename FileObj>
		std::optional<FileObj> parse_yaml_file(
		    std::filesystem::path const& path) {
			return parse_yaml_file_base<FileObj>(
			    [&](YAML& parser) { return parser.load(path); });
		}

		template <typename FileObj>
		std::optional<FileObj> parse_yaml_file_from_text(
		    std::string const& text,
		    std::string const& path) {
			return parse_yaml_file_base<FileObj>(
			    [&](YAML& parser) { return parser.load_contents(text, path); });
		}
	}  // namespace

	std::optional<config> config::parse_yaml(
	    std::filesystem::path const& path) {
		return parse_yaml_file<config>(path);
	}

	std::optional<std::map<std::chrono::year_month, currency>>
	config::parse_minimal_only(std::filesystem::path const& path) {
		return parse_yaml_file<std::map<std::chrono::year_month, currency>>(
		    path);
	}

	std::optional<std::map<std::chrono::year_month, currency>>
	config::parse_minimal_only_from_text(std::string const& text,
	                                     std::string const& path) {
		return parse_yaml_file_from_text<
		    std::map<std::chrono::year_month, currency>>(text, path);
	}

	bool config::validate() noexcept {
		if (!insurer.validate()) return false;
		for (auto& obj : insured) {
			if (!obj.validate()) return false;
		}
		return true;
	}

	template <typename Named>
	bool validate_name(Named& named) noexcept {
		auto const name = split_sv(", "sv, named.last_name, 1);
		if (name.size() != 2) return false;
		named.first_name = strip_sv(name[1]);
		named.last_name = strip_sv(name[0]);
		return !(named.last_name.empty() || named.first_name.empty());
	}

	bool person::validate() noexcept { return true; }

	bool insurer_t::postprocess() {
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

		return validate_name(*this) && !(social_id.empty() || tax_id.empty() ||
		                                 kind.empty() || document.empty());
	}

	bool insured_t::postprocess() {
		if (!validate_name(*this)) return false;

		if (title.code.length() != 8) return false;
		if (!(std::isdigit(title.code[0]) && std::isdigit(title.code[1]) &&
		      std::isdigit(title.code[2]) && std::isdigit(title.code[3]) &&
		      std::isdigit(title.code[5]) && std::isdigit(title.code[7]) &&
		      title.code[4] == ' ' && title.code[6] == ' ')) {
			return false;
		}

		auto const view = std::string_view{title.code};
		title.code = fmt::format("{}{}{}", view.substr(0, 4), view[5], view[7]);

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
		return parse_yaml_file<templates>(path);
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

	contribution rate::contribution_on(currency amount) const noexcept {
		return contribution_on(amount.calc());
	}

	contribution rate::contribution_on(calc_currency amount) const noexcept {
		auto const total_contribution = (amount * total).rounded();
		if (!insured) {
			return {.payer = total_contribution, .insured = {}};
		}
		auto const insured_contribution = (amount * *insured).rounded();
		return {.payer = total_contribution - insured_contribution,
		        .insured = insured_contribution};
	}
}  // namespace quick_dra::v1
