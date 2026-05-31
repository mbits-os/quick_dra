// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QMessageBox>
#include <app/main/options.hpp>
#include <cstdio>
#include <map>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/base/str.hpp>
#include <quick_dra/version.hpp>
#include <string>
#include <utility>

namespace args {
	struct string_printer {
		string_printer(std::string& out) : out{out} {}

		void print(char const* cur, size_t len) noexcept { out.append(cur, len); }
		void putc(char c) noexcept { out.push_back(c); }
		size_t width() const noexcept { return 0; }

	private:
		std::string& out;
	};

	template <>
	struct printer_base<string_printer> : printer_base_impl<string_printer> {
		using printer_base_impl<string_printer>::printer_base_impl;
		using printer_base_impl<string_printer>::format_paragraph;
		using printer_base_impl<string_printer>::format_list;
		inline void format_paragraph(std::string const& text, size_t indent, std::optional<size_t> maybe_width = {}) {
			format_paragraph(text, indent, maybe_width.value_or(width()));
		}
		inline void format_list(fmt_list const& info, std::optional<size_t> maybe_width = {}) {
			format_list(info, maybe_width.value_or(width()));
		}
	};
	using formatter = printer_base<string_printer>;
};  // namespace args

namespace quick_dra {
	namespace {
		void show_help(args::parser& parser) {
			std::string out;

			auto commands = parser.printer_arguments();

			auto shrt = "usage: "s;
			parser.printer_append_usage(shrt);
			args::formatter{out}.format_paragraph(shrt, 7, 80);
			args::formatter{out}.format_list(commands, 80);

			QMessageBox msgBox{
			    QMessageBox::Information,
			    QString{"Quick-DRA %1"}.arg(version::ui),
			    QString{"```\n%1\n```"}.arg(out),
			    QMessageBox::Ok,
			};
			msgBox.setTextFormat(Qt::MarkdownText);
			msgBox.setDefaultButton(QMessageBox::Ok);
			msgBox.exec();

			throw exit_on_help{};
		}
	}  // namespace

	options options::parse(args::args_view const& arguments) {
		std::optional<std::string> config_path;
		std::optional<std::filesystem::path> tax_config_path;

		args::null_translator tr{};
		args::parser parser{"show a GUI for configuration and KEDU generation"s, arguments, &tr};

		// TODO: GUI --help
		parser.provide_help(false);
		parser.custom(show_help, "h", "help").help("shows this help message and exits").opt();
		parser.arg(config_path, "config").meta("<path>").help("select config file; defaults to ~/.quick_dra.yaml");
		parser.arg(tax_config_path, "tax-config")
		    .meta("<path>")
		    .help(
		        "provide tax parameters file; will take precedent before data "
		        "from repository and installation");

		parser.parse();

		return {
		    .cfg_path = platform::get_config_path(config_path),
		    .tax_config_path = std::move(tax_config_path),
		};
	}
}  // namespace quick_dra
