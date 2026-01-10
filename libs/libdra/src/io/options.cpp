// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

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
		static constexpr auto GITHUB_MINIMAL_PATH_1 =
		    "libs/libdra/data/config/minimal_pay.yaml"sv;
		static constexpr auto GITHUB_MINIMAL_PATH_2 =
		    "data/config/minimal_pay.yaml"sv;

		currency find_minimal(year_month const& key,
		                      std::map<year_month, currency> const& minimal) {
			year_month result_date{};
			currency result{};

			for (auto const& [date, amount] : minimal) {
				if (key < date) continue;
				if (result_date > date) continue;
				result_date = date;
				result = amount;
			}

			return result;
		}

		std::optional<std::map<year_month, currency>> download_minimal_option(
		    verbose level,
		    std::string_view path) {
			std::string url{};
			url = GITHUB_MAIN_BRANCH;
			url += path;

			auto resp = http_get(url);
			if (!resp) {
				return {};
			}

			auto view = resp.text();

			// TODO: object vs array
			auto result = config::parse_minimal_only_from_text(
			    {view.data(), view.size()}, url);
			if (level >= verbose::parameters) {
				fmt::print("-- downloaded {}\n", url);
			}
			return result;
		}

		std::optional<std::map<year_month, currency>> download_minimal(
		    verbose level) {
			auto result = download_minimal_option(level, GITHUB_MINIMAL_PATH_1);
			if (result) return result;
			return download_minimal_option(level, GITHUB_MINIMAL_PATH_2);
		}

		using minimal_loader = decltype(download_minimal)*;
	}  // namespace

	std::string set_filename(unsigned report_index, year_month const& date) {
		return fmt::format("quick-dra_{}{:02}-{:02}.xml",
		                   static_cast<int>(date.year()),
		                   static_cast<unsigned>(date.month()), report_index);
	}

	std::optional<config> parse_config(verbose level,
	                                   year_month const& date,
	                                   std::filesystem::path const& path) {
		auto result = config::parse_yaml(path);
		if (!result) return result;

		static constexpr minimal_loader loaders[] = {
		    download_minimal,
		    +[](verbose) {
			    return config::parse_minimal_only(platform::config_data_dir() /
			                                      "minimal_pay.yaml"sv);
		    },
		};

		for (auto const& loader : loaders) {
			auto minimal = loader(level);

			if (minimal) {
				result->minimal.merge(*minimal);
			}
		}

		result->params.minimal_pay = find_minimal(date, result->minimal);
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
				fmt::print("-- minimal pay for month reported: {:.02f} zł\n",
				           result->params.minimal_pay);
			}
		}

		return result;
	}
}  // namespace quick_dra
