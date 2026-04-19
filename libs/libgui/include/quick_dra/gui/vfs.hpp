// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <map>
#include <optional>
#include <quick_dra/gui/fs_common.hpp>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

using namespace std::literals;

namespace quick_dra::gui {
	struct directory_entry;

	using directory = std::map<std::u8string, directory_entry>;
	using file_contents = std::span<char const>;

	struct entry {
		std::string_view name{};
		file_contents contents{};
	};

	struct directory_entry : std::variant<file_contents, directory> {
		directory* parent = nullptr;

		using base_type = std::variant<file_contents, directory>;
		using base_type::base_type;

		inline bool is_file() const noexcept { return std::holds_alternative<file_contents>(base()); }

		inline bool is_dir() const noexcept { return std::holds_alternative<directory>(base()); }

		inline base_type const& base() const noexcept { return *this; }
	};

	class virtual_filesystem : public basic_filesystem<virtual_filesystem> {
	public:
		static std::string_view tag() noexcept { return "vfs"sv; }

		static virtual_filesystem build(std::span<entry const> const& files);

		directory const& root() const noexcept { return root_; }

		directory_entry const* locate(std::string_view path) const;
		std::optional<html_response> respond(std::string_view path, std::vector<char> const&) const;

		static void set_global(virtual_filesystem&& dir) noexcept;
		static virtual_filesystem const& get_global() noexcept;
		static void install_global_data();

	private:
		directory root_;
	};
}  // namespace quick_dra::gui
