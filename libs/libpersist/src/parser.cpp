// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <fmt/std.h>
#include <fstream>
#include <optional>
#include <string>
#include <utility>
#include <yaml/parser.hpp>

using namespace std::literals;

namespace yaml {
	namespace {
		template <typename ErrorCallback>
		std::optional<std::string> open_with(std::filesystem::path const& path,
		                                     ErrorCallback&& on_error) {
			std::optional<std::string> result{};
			std::ifstream in{path, std::ios::in | std::ios::binary};
			if (!in) {
				on_error();
				return result;
			}

			std::ostringstream contents;
			contents << in.rdbuf();
			result = std::move(contents).str();

			return result;
		}

		std::optional<std::string> open(std::filesystem::path const& path,
		                                std::string_view app_name) {
			return open_with(path, [app_name, &path] {
				fmt::print("{}: error: cannot find {}\n", app_name, path);
			});
		}
	}  // namespace

	base_ctx parser::context(base_ctx const& parent) const {
		auto result = parent;
		result.parser = &rapid_parser;
		return result;
	}

	ryml::Parser parser::build_rapid_parser() noexcept {
		ryml::Parser p{&evt_handler, ryml::ParserOptions{}.locations(true)};
		p.reserve_locations(300);
		return p;
	}

	std::optional<ryml::Tree> parser::load(std::filesystem::path const& path,
	                                       std::string_view app_name) & {
		auto maybe_contents = open(path, app_name);
		if (!maybe_contents) {
			return std::nullopt;
		}

		return load_contents(std::move(*maybe_contents), path.string());
	}

	std::optional<ryml::Tree> parser::load(
	    std::filesystem::path const& path,
	    std::function<void()> const& on_error) & {
		auto maybe_contents = open_with(path, on_error);
		if (!maybe_contents) {    // GCOV_EXCL_LINE
			return std::nullopt;  // GCOV_EXCL_LINE
		}  // GCOV_EXCL_LINE

		return load_contents(std::move(*maybe_contents), path.string());
	}

	std::optional<ryml::Tree> parser::load_contents(std::string text,
	                                                std::string const& path) & {
		contents = std::move(text);
		path_str = path;
		rapid_parser.reserve_locations(300);
		auto tree =
		    ryml::parse_in_place(&rapid_parser, ryml::to_csubstr(path_str),
		                         ryml::to_substr(contents));
		tree.resolve();
		return std::optional{std::move(tree)};
	}
}  // namespace yaml
