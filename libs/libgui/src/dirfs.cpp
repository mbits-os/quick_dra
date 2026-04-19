// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <array>
#include <cstdio>
#include <memory>
#include <quick_dra/base/str.hpp>
#include <quick_dra/gui/dirfs.hpp>
#include <utility>
#include <vector>

using namespace std::literals;

namespace quick_dra::gui {
	namespace {
		struct file_closer {
			void operator()(FILE* f) { std::fclose(f); }
		};

		using file_ptr = std::unique_ptr<FILE, file_closer>;

		directory_filesystem& global_dirfs() noexcept {
			static directory_filesystem vfs{};
			return vfs;
		}

		inline bool is_relative_to(std::filesystem::path const& child, std::filesystem::path const& parent) {
			if (!child.native().starts_with(parent.native())) return false;
			auto const parent_size = parent.native().length();
			return child.native().length() == parent_size ||
			       child.native()[parent_size] == std::filesystem::path::preferred_separator;
		}
	}  // namespace

	void directory_filesystem::set_global(std::filesystem::path const& root) noexcept {
		gui::global_dirfs().root_ = std::filesystem::weakly_canonical(root);
		fmt::print("[dirfs] Serving {}\n", as_str(gui::global_dirfs().root_.u8string()));
	}

	directory_filesystem const& directory_filesystem::get_global() noexcept { return gui::global_dirfs(); }

	std::optional<html_response> directory_filesystem::respond(std::string_view filename,
	                                                           std::vector<char>& stg) const {
		stg.clear();
		auto const original_filename = filename;
		while (filename.starts_with('/')) {
			filename = filename.substr(1);
		}

		std::optional<html_response> result = std::nullopt;

		auto const path = weakly_canonical(root_ / std::filesystem::path{as_u8v(filename)});

		if (!is_relative_to(path, root_)) {
			return result;
		}

		auto const is_dir = std::filesystem::is_directory(path);

		if (is_dir && !original_filename.ends_with('/')) {
			result = html_response{
			    .contents = "The file is elsewhere"sv,
			    .content_type = "text/html"sv,
			    .redirect = fmt::format("{}/", original_filename),
			};
			return result;
		}

		if (is_dir) {
			auto const path_index_html = fmt::format("{}index.html", original_filename);
			result = respond(path_index_html, stg);
			return result;
		}

#ifdef _WIN32
		auto const file = [&path]() {
			FILE* ptr{};
			_wfopen_s(&ptr, path.native().c_str(), L"rb");
			return file_ptr{ptr};
		}();
#else
		auto const file = file_ptr{std::fopen(path.native().c_str(), "rb")};
#endif

		char buffer[8192];
		while (auto const size = std::fread(buffer, 1, sizeof(buffer), file.get())) {
			stg.insert(stg.end(), buffer, buffer + size);
		}

		auto const s_file = as_str(filename);
		result = html_response{
		    .contents = {stg.data(), stg.size()},
		    .content_type = webui_get_mime_type(s_file.c_str()),
		};
		return result;
	}
}  // namespace quick_dra::gui
