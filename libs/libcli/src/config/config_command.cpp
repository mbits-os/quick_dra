// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/std.h>
#include <array>
#include <filesystem>
#include <quick_dra/base/chrono.hpp>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/base/str.hpp>
#include <quick_dra/cli/builtins.hpp>
#include <quick_dra/conv/args_parser.hpp>
#include <quick_dra/conv/conversation.hpp>
#include <quick_dra/conv/low_level.hpp>
#include <quick_dra/conv/search.hpp>
#include <quick_dra/models/project_reader.hpp>
#include <quick_dra/models/types.hpp>
#include <string>
#include <vector>

namespace quick_dra::builtin::config {
	namespace {
#define BUILTINS_X_KNOWN(NAME, TOOL, DSCR) help_command{TOOL##sv, DSCR##sv},
		static constexpr auto config_commands = std::array{CONFIG_BUILTINS_X(BUILTINS_X_KNOWN)};
#undef BUILTINS_X_KNOWN

		static constexpr auto command_groups = std::array{
		    help_group{"known commands"sv, config_commands},
		};

		static constexpr auto months = std::array{
		    "January"sv, "February"sv, "March"sv,     "April"sv,   "May"sv,      "June"sv,
		    "July"sv,    "August"sv,   "September"sv, "October"sv, "November"sv, "December"sv,
		};

		std::string fmt_date(year_month const& ym) {
			if (ym == null_month) {
				return "\"default\" month"s;
			}
			if (ym.month().ok() && ym.year() > 0y) {
				return std::format("{} {}", months[static_cast<unsigned>(ym.month()) - 1], static_cast<int>(ym.year()));
			}
			// GCOV_EXCL_START
			// Currently, there is no way of entering this line -- both command line and YAML reader code bailing on
			// invalid dates long before we even approach this function. _However_, this stays here as fallback from
			// neutrinos frying the RAM cell ;)
			// These guards can be removed once the function is moved to base/chrome.cpp and becomes properly testable
			return std::format("{:04}/{:02}", static_cast<int>(ym.year()), static_cast<unsigned>(ym.month()));
			// GCOV_EXCL_STOP
		}

		void upgrade_v2(partial::config& cfg, year_month const& start_of_history) {
			for (auto& insured : *cfg.insured) {
				auto it = insured.history->begin();
				auto employment = it->second;
				insured.history->erase(it);
				(*insured.history)[start_of_history] = employment;
			}
		}

		bool upgrade_all(partial::config& cfg, year_month const& start_of_history) {
			if (cfg.version == kApiVersion) return false;

			if (cfg.version < v2::kApiVersion) upgrade_v2(cfg, start_of_history);
			// if (cfg.version < v3::kApiVersion) ...

			cfg.version = kApiVersion;
			fmt::print("\033[0;90mConfig version updated\033[m\n");

			return true;
		}
	}  // namespace

	int handle(std::string_view tool_name, args::arglist arguments, std::string_view description) {
		builtin::parser parser{description, {tool_name, arguments}, command_groups};

		return quick_dra::tools::run(parser, config::tools, tool_name);
	}

	namespace upgrade {
		int handle(std::string_view tool_name, args::arglist arguments, std::string_view description) {
			std::optional<std::string> config_path;
			std::optional<std::string> changes_date{};

			args::null_translator tr{};
			args::parser parser{as_str(description), {tool_name, arguments}, &tr};

			parser.arg(config_path, "config").meta("<path>").help("select config file; defaults to ~/.quick_dra.yaml");
			parser.arg(changes_date, "on")
			    .meta("<yyyy/mm>")
			    .help("select month as start of employment, when upgrading from v1 to v2");

			parser.parse();

			auto month = null_month;
			if (changes_date) {
				if (!yaml::convert_string(*changes_date, month)) {
					parser.error(fmt::format("--on expected YYYY/MM, got `{}`", *changes_date));
				}
			} else {
				month = last_date_or_today<int>(month, {});
			}

			auto const path = platform::get_config_path(config_path);
			auto cfg = partial::config::load_partial(path);
			if (!cfg.accident_insurance) {
				cfg.accident_insurance.emplace();
			}

			if (upgrade_all(cfg, month)) {
				if (!cfg.store(path)) {
					// GCOV_EXCL_START
					fmt::print(stderr, "Quick-DRA: error: could not write to {}\n", path);
					return 1;
				}  // GCOV_EXCL_STOP
			}

			return 0;
		}
	}  // namespace upgrade

	namespace accident_insurance {
		int handle(std::string_view tool_name, args::arglist arguments, std::string_view description) {
			std::optional<std::string> config_path;
			std::optional<std::string> changes_date{};
			std::optional<percent> contribution{};

			args::null_translator tr{};
			args::parser parser{as_str(description), {tool_name, arguments}, &tr};

			parser.arg(config_path, "config").meta("<path>").help("select config file; defaults to ~/.quick_dra.yaml");
			parser.arg(changes_date, "on").meta("<yyyy/mm>").help("which month the contribution should refer to");
			parser.arg(contribution)
			    .meta("<percent>")
			    .help("update contribution to this value is preset; show contribution value if absent");

			parser.parse();

			auto month = null_month;
			if (changes_date) {
				if (!yaml::convert_string(*changes_date, month)) {
					parser.error(fmt::format("--on expected YYYY/MM, got `{}`", *changes_date));
				}
			} else {
				month = last_date_or_today<int>(month, {});
			}

			auto const path = platform::get_config_path(config_path);
			auto cfg = partial::config::load_partial(path);
			if (!cfg.accident_insurance) {
				cfg.accident_insurance.emplace();
			}

			if (contribution) {
				(*cfg.accident_insurance)[month] = *contribution;
				fmt::print("\033[0;90mAccident contribution for {} set to \033[m{}%\n", fmt_date(month), *contribution);

				upgrade_all(cfg, month);

				if (!cfg.store(path)) {
					// GCOV_EXCL_START
					fmt::print(stderr, "Quick-DRA: error: could not write to {}\n", path);
					return 1;
				}  // GCOV_EXCL_STOP
				return 0;
			}

			auto const [origin, selected_contribution] = find_in_timeline(month, *cfg.accident_insurance);
			if (origin != null_month || selected_contribution != 0_per) {
				fmt::print("{}% \033[0;90m[set in {}]\033[m\n", selected_contribution, fmt_date(origin));
			}

			return 0;
		}
	}  // namespace accident_insurance
}  // namespace quick_dra::builtin::config
