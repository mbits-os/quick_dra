// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/std.h>
#include <map>
#include <optional>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/io/http.hpp>
#include <quick_dra/io/options.hpp>
#include <quick_dra/version.hpp>

namespace quick_dra {
	namespace {
		static constexpr auto GITHUB_MAIN_BRANCH =
		    "https://raw.githubusercontent.com/mbits-os/quick_dra/refs/heads/main/"sv;
		static constexpr auto GITHUB_TAX_CONFIG =
		    "data/config/tax_config.yaml"sv;

		template <typename T>
		T find_in_timeline(year_month const& key,
		                   std::map<year_month, T> const& minimal) {
			year_month result_date{};
			T result{};

			for (auto const& [date, amount] : minimal) {
				if (key < date) continue;
				if (result_date > date) continue;
				result_date = date;
				result = amount;
			}

			return result;
		}

		std::optional<tax_config> download_tax_config(verbose level) {
			std::string url{};
			url = GITHUB_MAIN_BRANCH;
			url += GITHUB_TAX_CONFIG;

			auto resp = http_get(url);
			if (!resp) {
				if (level >= verbose::parameters) {
					fmt::print("-- nothing downloaded (status: {}, {})\n",
					           resp.status, url);
				}
				return {};
			}

			auto view = resp.text();

			// TODO: object vs array
			auto result =
			    tax_config::parse_from_text({view.data(), view.size()}, url);
			if (level >= verbose::parameters) {
				fmt::print("-- downloaded {}\n", url);
			}
			return result;
		}

		using tax_config_loader = std::function<decltype(download_tax_config)>;
	}  // namespace

	std::string set_filename(unsigned report_index, year_month const& date) {
		return fmt::format("quick-dra_{}{:02}-{:02}.xml",
		                   static_cast<int>(date.year()),
		                   static_cast<unsigned>(date.month()), report_index);
	}

	std::optional<config> parse_config(
	    verbose level,
	    year_month const& date,
	    std::filesystem::path const& path,
	    std::optional<std::filesystem::path> tax_config_path) {
		auto result = config::parse_yaml(path);
		if (!result) return result;

		tax_config_loader const loaders[] = {
		    [&tax_config_path](verbose) -> std::optional<tax_config> {
			    if (!tax_config_path) return std::nullopt;
			    return tax_config::parse_yaml(*tax_config_path);
		    },
		    download_tax_config,
		    +[](verbose) {
			    return tax_config::parse_yaml(platform::config_data_dir() /
			                                  "tax_config.yaml"sv);
		    },
		};

		tax_config tax_cfg{};
		auto read_one_tax_config = false;
		for (auto const& loader : loaders) {
			auto current = loader(level);

			if (current) {
				read_one_tax_config = true;
				tax_cfg.merge(std::move(*current));
			}
		}

		if (!read_one_tax_config) {
			result.reset();
			return result;
		}

#define FIND_IN_TIMELINE(NAME) \
	result->params.NAME = find_in_timeline(date, tax_cfg.NAME)
		FIND_IN_TIMELINE(scale);
		FIND_IN_TIMELINE(minimal_pay);
		FIND_IN_TIMELINE(costs_of_obtaining);
		FIND_IN_TIMELINE(contributions);

		tax_cfg.debug_print(level);
		result->debug_print(level);

		if (level >= verbose::names_and_summary) {
			bool everyone_has_salary = true;
			for (auto const& insured : result->insured) {
				if (!insured.remuneration) {
					everyone_has_salary = false;
					break;
				}
			}
			if (!everyone_has_salary) {
				fmt::print("--   minimal pay for month reported: {:.02f} zł\n",
				           result->params.minimal_pay);
			}
		}

		return result;
	}
}  // namespace quick_dra
