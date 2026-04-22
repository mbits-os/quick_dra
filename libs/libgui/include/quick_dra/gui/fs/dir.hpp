// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <quick_dra/gui/fs/common.hpp>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

using namespace std::literals;

namespace quick_dra::gui {
	class directory_filesystem : public basic_filesystem<directory_filesystem> {
	public:
		directory_filesystem() = default;

		static std::string_view tag() noexcept { return "dirfs"sv; }  // GCOV_EXCL_LINE
		std::filesystem::path const& root() const noexcept { return root_; }
		std::optional<html_response> respond(std::string_view path, std::vector<char>& stg) const;
		static void set_global(std::filesystem::path const& root) noexcept;
		static directory_filesystem const& get_global() noexcept;
		static void install_global_data();

	private:
		explicit directory_filesystem(std::filesystem::path const& root) : root_{root} {}

		std::filesystem::path root_;
	};
}  // namespace quick_dra::gui
