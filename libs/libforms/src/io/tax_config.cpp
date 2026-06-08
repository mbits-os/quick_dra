// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/std.h>
#include <array>
#include <map>
#include <optional>
#include <quick_dra/base/chrono.hpp>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/io/http.hpp>
#include <quick_dra/io/options.hpp>
#include <quick_dra/io/tax_config.hpp>
#include <quick_dra/version.hpp>
#include <string>
#include <utility>

namespace quick_dra {
	namespace {
		static constexpr auto GITHUB_MAIN_BRANCH =
		    "https://raw.githubusercontent.com/mbits-os/quick_dra/refs/heads/main/"sv;
		static constexpr auto GITHUB_TAX_CONFIG = "data/config/tax_config.yaml"sv;

		std::optional<tax_config> download_tax_config(verbose level) {
			std::string url{};
			url = GITHUB_MAIN_BRANCH;
			url += GITHUB_TAX_CONFIG;

			auto resp = http_get(url);
			if (!resp) {
				if (level >= verbose::parameters) {
					fmt::print("-- nothing downloaded (status: {}, {})\n", resp.status, url);
				}
				return {};
			}

			auto view = resp.text();

			// TODO: object vs array
			auto result = tax_config::parse_from_text({view.data(), view.size()}, url);
			if (level >= verbose::parameters) {
				fmt::print("-- downloaded {}\n", url);
			}
			return result;
		}

		using tax_config_loader = std::function<decltype(download_tax_config)>;
	}  // namespace

	std::optional<tax_config> load_tax_config(verbose level,
	                                          std::optional<std::filesystem::path> const& tax_config_path,
	                                          github_config download) {
		auto const loaders = std::array{
		    // tax_config_loader
		    std::function{[&tax_config_path](verbose) -> std::optional<tax_config> {
			    if (!tax_config_path) return std::nullopt;
			    return tax_config::parse_yaml(*tax_config_path);
		    }},
		    std::function{[download](verbose level) -> std::optional<tax_config> {
			    if (download == github_config::download) return download_tax_config(level);
			    return std::nullopt;
		    }},
		    std::function{
		        +[](verbose) { return tax_config::parse_yaml(platform::config_data_dir() / "tax_config.yaml"sv); }},
		};

		std::optional<tax_config> result{};
		result.emplace();
		auto read_one_tax_config = false;
		for (auto const& loader : loaders) {
			auto current = loader(level);

			if (current) {
				read_one_tax_config = true;
				result->merge(std::move(*current));
			}
		}

		if (!read_one_tax_config) {
			result.reset();
		}

		return result;
	}

	void lookup_parameters(tax_parameters& out,
	                       tax_config const& config,
	                       std::optional<std::map<std::chrono::year_month, percent>> const& accident_insurance_override,
	                       year_month const& key) {
#define FIND_IN_TIMELINE(NAME) out.NAME = find_in_timeline(key, config.NAME).second
		FIND_IN_TIMELINE(scale);
		FIND_IN_TIMELINE(minimal_pay);
		FIND_IN_TIMELINE(costs_of_obtaining);
		FIND_IN_TIMELINE(contributions);

		if (accident_insurance_override && !accident_insurance_override->empty()) {
			out.contributions.accident_insurance = {
			    .payer = find_in_timeline(key, *accident_insurance_override).second,
			};
		}
	}
}  // namespace quick_dra
