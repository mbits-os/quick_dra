// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <filesystem>
#include <ryml.hpp>
#include <ryml_std.hpp>
#include <string>
#include <string_view>
#include <yaml/reader.hpp>
#include <yaml/ref.hpp>

namespace yaml {
	struct parser {
		base_ctx context(base_ctx const& parent = {}) const;
		ryml::Parser build_rapid_parser() noexcept;

		ryml::EventHandlerTree evt_handler{};
		ryml::Parser rapid_parser{build_rapid_parser()};
		std::string contents{};
		std::string path_str{};

		parser() = default;
		parser(parser const&) = delete;
		parser(parser&&) = delete;

		template <typename FileObj>
		static std::optional<FileObj> parse_yaml_file(
		    std::filesystem::path const& path,
		    std::string_view app_name) {
			return parse_yaml<FileObj>(
			    [&](parser& parser) { return parser.load(path, app_name); });
		}

		template <typename FileObj>
		static std::optional<FileObj> parse_yaml_text(std::string const& text,
		                                              std::string const& path) {
			return parse_yaml<FileObj>([&](parser& parser) {
				return parser.load_contents(text, path);
			});
		}

	private:
		std::optional<ryml::Tree> load(std::filesystem::path const& path,
		                               std::string_view app_name) &;
		std::optional<ryml::Tree> load_contents(std::string text,
		                                        std::string const& path) &;

		template <typename T>
		    requires requires(T& obj) {
			    { obj.validate() } -> std::same_as<bool>;
		    }
		static bool validate(T& obj) {
			return obj.validate();
		}
		static bool validate(auto const&) { return true; }

		template <typename FileObj, typename Callback>
		    requires requires(Callback&& cb, parser& p) {
			    { cb(p) } -> std::same_as<std::optional<ryml::Tree>>;
		    }
		static std::optional<FileObj> parse_yaml(Callback&& cb) {
			std::optional<FileObj> result{};

			try {
				ref_ctx::error_handler handler{};
				handler.install_in_c4();

				parser storage{};

				auto maybe_tree = cb(storage);
				if (!maybe_tree || handler.failed()) {
					return result;
				}
				auto& tree = *maybe_tree;

				result.emplace();

				auto root = tree.rootref();

				if (!read_value(storage.context().from(root), *result) ||
				    !validate(*result)) {
					result.reset();
				}
			} catch (c4_error_exception const&) {
				result.reset();
			}

			return result;
		}
	};
}  // namespace yaml
