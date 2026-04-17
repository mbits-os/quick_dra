// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/std.h>
#include <gtest/gtest.h>
#include <filesystem>
#include <optional>
#include <quick_dra/base/str.hpp>
#include <quick_dra/gui/vfs.hpp>
#include <source_location>
#include <span>
#include <variant>

#undef stdout
#undef stderr

namespace quick_dra::gui::testing {
	void print(directory const& dir, size_t level = 0) {
		for (auto const& [name, entry] : dir) {
			if (entry.is_file()) {
				continue;
			}
			fmt::print("{:{}}{}/\n", "", level * 2, as_sv(name));
			print(std::get<directory>(entry), level + 1);
		}
		for (auto const& [name, entry] : dir) {
			if (entry.is_file()) {
				fmt::print("{:{}}{}\n", "", level * 2, as_sv(name));
			}
		}
	}

	void print(virtual_filesystem const& fs) { print(fs.root()); }

	bool has_entry(directory const& dir, std::u8string const& name, bool file) {
		auto const it = dir.find(name);
		if (it == dir.end()) {
			return false;
		}
		return it->second.is_file() == file;
	}

	bool has_file(directory const& dir, std::u8string const& name) { return has_entry(dir, name, true); }

	bool has_dir(directory const& dir, std::u8string const& name) { return has_entry(dir, name, false); }

	TEST(vfs, build_map) {
		static constexpr entry files[] = {
		    {"/index.html"sv, {}},
		    {"/assets//style.css"sv, {}},
		    {"/assets/style.css/not-possible"sv, {}},
		    {"//assets/logo.png"sv, {}},
		};

		auto actual = virtual_filesystem::build(files);

		EXPECT_EQ(actual.root().size(), 2);
		EXPECT_TRUE(has_file(actual.root(), u8"index.html"s));
		ASSERT_TRUE(has_dir(actual.root(), u8"assets"s));

		auto& assets = std::get<directory>(*actual.locate("assets"sv));
		EXPECT_EQ(assets.size(), 2);
		EXPECT_TRUE(has_file(assets, u8"style.css"s));
		EXPECT_TRUE(has_file(assets, u8"logo.png"s));

		print(actual);
	}

	auto web_app() {
		static constexpr entry const files[] = {
		    {"index.html"sv, "root index"sv},
		    {"sub/index.html", "sub index"sv},
		    {"assets/style.css"sv, "style"sv},
		    {"assets/logo.png"sv, "logo"sv},
		    {"strange-sub/index.html/should-not-be-a-dir"sv, {}},
		};

		return virtual_filesystem::build(files);
	}

	TEST(vfs, locate) {
		auto const fs = web_app();
		EXPECT_NE(fs.locate("/index.html"sv), nullptr);
		EXPECT_NE(fs.locate("index.html"sv), nullptr);
		EXPECT_NE(fs.locate("sub"sv), nullptr);
		ASSERT_NE(fs.locate("sub/"sv), nullptr);
		ASSERT_NE(fs.locate("sub/index.html"sv), nullptr);
		ASSERT_NE(fs.locate("assets/logo.png"sv), nullptr);

		EXPECT_EQ(fs.locate("sub2"sv), nullptr);
		EXPECT_EQ(fs.locate("/sub2/index.html"sv), nullptr);
		EXPECT_EQ(fs.locate("assets/logo.jpg"sv), nullptr);

		EXPECT_TRUE(fs.locate("sub"sv)->is_dir());
		EXPECT_TRUE(fs.locate("sub/"sv)->is_dir());
		EXPECT_TRUE(fs.locate("sub/index.html"sv)->is_file());
	}

	TEST(vfs, respond) {
		auto const fs = web_app();
		EXPECT_EQ(fs.respond("/index.html"sv)->contents, "root index"sv);
		EXPECT_EQ(fs.respond("index.html"sv)->contents, "root index"sv);
		EXPECT_EQ(fs.respond("sub"sv)->redirect, "sub/"sv);
		EXPECT_EQ(fs.respond("sub/"sv)->contents, "sub index"sv);
		EXPECT_EQ(fs.respond("sub/"sv)->content_type, "text/html"sv);
		EXPECT_EQ(fs.respond("sub/index.html"sv)->contents, "sub index"sv);
		EXPECT_EQ(fs.respond("assets/logo.png"sv)->contents, "logo"sv);
		EXPECT_EQ(fs.respond("assets/logo.png"sv)->content_type, "image/png"sv);
		EXPECT_FALSE(fs.respond("assets/"sv));
		EXPECT_FALSE(fs.respond("strange-sub/"sv));
	}

	std::string unwrap(webui::ptr<char> const& payload, int size) {
		if (!payload) {
			return "404"s;
		}

		return {payload.get(), static_cast<size_t>(size)};
	}

	std::string unwrapped_http_response(virtual_filesystem const& fs, std::string_view path) {
		int size{};
		auto payload = fs.http_response(path, size);
		return unwrap(payload, size);
	}

	TEST(vfs, http_response) {
		auto const fs = web_app();
		print(fs);

		EXPECT_EQ(unwrapped_http_response(fs, "/index.html"sv),
		          "HTTP/1.1 200 OK\r\n"
		          "Content-Type: text/html\r\n"
		          "Content-Length: 10\r\n"
		          "Cache-Control: no-cache\r\n"
		          "\r\n"
		          "root index"sv);
		EXPECT_EQ(unwrapped_http_response(fs, "index.html"sv),
		          "HTTP/1.1 200 OK\r\n"
		          "Content-Type: text/html\r\n"
		          "Content-Length: 10\r\n"
		          "Cache-Control: no-cache\r\n"
		          "\r\n"
		          "root index"sv);
		EXPECT_EQ(unwrapped_http_response(fs, "sub"sv),
		          "HTTP/1.1 302 Found\r\n"
		          "Location: sub/\r\n"
		          "Content-Type: text/html\r\n"
		          "Content-Length: 21\r\n"
		          "Cache-Control: no-cache\r\n"
		          "\r\n"
		          "The file is elsewhere"sv);
		EXPECT_EQ(unwrapped_http_response(fs, "sub/"sv),
		          "HTTP/1.1 200 OK\r\n"
		          "Content-Type: text/html\r\n"
		          "Content-Length: 9\r\n"
		          "Cache-Control: no-cache\r\n"
		          "\r\n"
		          "sub index"sv);
	}

	TEST(vfs, global_vfs_handler) {
		static constexpr auto CANARY = 0xBADF00D;

		virtual_filesystem::set_global(web_app());

		int size = CANARY;
		webui::ptr<char> response{
		    reinterpret_cast<char*>(const_cast<void*>(virtual_filesystem::global_handler("assets/style.css", &size)))};

		ASSERT_TRUE(response);
		ASSERT_EQ(size, 92);
		ASSERT_EQ(std::string_view(response.get(), static_cast<size_t>(size)),
		          "HTTP/1.1 200 OK\r\n"
		          "Content-Type: text/css\r\n"
		          "Content-Length: 5\r\n"
		          "Cache-Control: no-cache\r\n"
		          "\r\n"
		          "style"sv);

		size = CANARY;
		response = webui::ptr<char>{
		    reinterpret_cast<char*>(const_cast<void*>(virtual_filesystem::global_handler("assets/logo.jpg", &size)))};

		ASSERT_FALSE(response);
		ASSERT_EQ(size, CANARY);
	}

	TEST(vfs, tarball) {
		virtual_filesystem::install_global_data();
		auto const fs = virtual_filesystem::get_global();
		ASSERT_TRUE(fs.respond("/index.html"sv));
	}
}  // namespace quick_dra::gui::testing