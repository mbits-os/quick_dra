// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <filesystem>
#include <quick_dra/base/str.hpp>
#include <quick_dra/gui/vfs.hpp>
#include <utility>
#include <vector>

namespace quick_dra::gui {
	namespace {
		virtual_filesystem& global_vfs() noexcept {
			static virtual_filesystem vfs{};
			return vfs;
		}

		inline auto empty_dir(directory* parent) {
			auto result = directory_entry{directory{}};
			result.parent = parent;
			return result;
		}
		inline auto empty_file(directory* parent) {
			auto result = directory_entry{file_contents{}};
			result.parent = parent;
			return result;
		}
		template <typename T>
		inline T const& as_const(T const& obj) noexcept {
			return obj;
		}

		bool add_file(directory& result, entry const& file) {
			auto entryname = file.name;
			while (entryname.starts_with('/')) {
				entryname = entryname.substr(1);
			}
			auto path = std::filesystem::path{as_u8v(entryname)};

			auto const dirname = path.parent_path();

			auto* dir = &result;
			for (auto const& filename : dirname) {
				auto const name = filename.u8string();
				if (name.empty()) continue;

				auto it = dir->lower_bound(name);
				if (it == dir->end() || it->first != name) {
					it = dir->insert(it, std::pair{as_const(name), empty_dir(dir)});
				}

				if (!it->second.is_dir()) return false;
				dir = &std::get<directory>(it->second);
			}

			auto const name = path.filename().u8string();
			if (name.empty()) return true;

			auto it = dir->lower_bound(name);
			if (it == dir->end() || it->first != name) {
				it = dir->insert(it, std::pair{as_const(name), empty_file(dir)});
			}

			if (!it->second.is_file()) return false;
			auto& contents = std::get<file_contents>(it->second);
			contents = file.contents;
			return true;
		}
	}  // namespace

	void virtual_filesystem::set_global(virtual_filesystem&& dir) noexcept { gui::global_vfs() = std::move(dir); }

	virtual_filesystem const& virtual_filesystem::get_global() noexcept { return gui::global_vfs(); }

	virtual_filesystem virtual_filesystem::build(std::span<entry const> const& files) {
		virtual_filesystem result{};

		for (auto const& file : files) {
			add_file(result.root_, file);
		}

		return result;
	}  // GCOV_EXCL_LINE[GCC]

	directory_entry const* virtual_filesystem::locate(std::string_view file) const {
		while (file.starts_with('/')) {
			file = file.substr(1);
		}
		auto path = std::filesystem::path{as_u8v(file)};

		auto const dirname = path.parent_path();

		auto* dir = &root_;
		directory_entry const* current_dir = nullptr;
		for (auto const& filename : dirname) {
			auto const name = filename.u8string();
			if (name.empty()) continue;

			auto it = dir->find(name);
			if (it == dir->end()) {
				return nullptr;
			}

			if (!it->second.is_dir()) return nullptr;
			current_dir = &it->second;
			dir = &std::get<directory>(it->second);
		}

		auto const name = path.filename().u8string();
		if (name.empty()) {
			return current_dir;
		}

		auto it = dir->find(name);
		if (it == dir->end()) {
			return nullptr;
		}

		return &it->second;
	}

	std::optional<html_response> virtual_filesystem::respond(std::string_view path, std::vector<char> const&) const {
		std::optional<html_response> result = std::nullopt;
		auto entry = locate(path);

		if (!entry) {
			return result;
		}

		if (entry->is_dir() && !path.ends_with('/')) {
			result = html_response{
			    .contents = "The file is elsewhere"sv,
			    .content_type = "text/html"sv,
			    .redirect = fmt::format("{}/", path),
			};
			return result;
		}

		if (entry->is_dir()) {
			auto& dir = std::get<directory>(*entry);
			auto const it = dir.find(u8"index.html"s);
			entry = it == dir.end() ? nullptr : &it->second;

			if (!entry || entry->is_dir()) {
				return result;
			}

			path = "index.html"sv;
		}

		auto& contents = std::get<file_contents>(*entry);

		auto const s_path = as_str(path);
		result = html_response{
		    .contents = {contents.data(), contents.size()},
		    .content_type = webui_get_mime_type(s_path.c_str()),
		};
		return result;
	}
}  // namespace quick_dra::gui
