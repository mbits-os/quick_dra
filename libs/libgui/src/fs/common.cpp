// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <filesystem>
#include <quick_dra/base/str.hpp>
#include <quick_dra/gui/fs/common.hpp>
#include <utility>

namespace quick_dra::gui {
	namespace {
		webui::ptr<char> wrap(std::string_view view, int& size) {
			auto result = webui::make_ptr<char>(view.size());
			memcpy(result.get(), view.data(), view.size());
			size = static_cast<int>(view.size());
			return result;
		}
	}  // namespace

	webui::ptr<char> wrap_response(std::optional<html_response> const& response,
	                               std::string_view tag,
	                               std::string_view path,
	                               int& size) {
		if (!response) {
			fmt::print("[{}] 404 {}\n", tag, path);
			return nullptr;
		}

		if (response->redirect) {
			fmt::print("[{}] 302 {} -> {}\n", tag, path, *response->redirect);
			return wrap(
			    fmt::format("HTTP/1.1 302 Found\r\n"
			                "Location: {}\r\n"
			                "Content-Type: {}\r\n"
			                "Content-Length: {}\r\n"
			                "Cache-Control: no-cache\r\n"
			                "\r\n"
			                "{}",
			                *response->redirect, response->content_type, response->contents.size(), response->contents),
			    size);
		}
		fmt::print("[{}] 200 {}\n", tag, path);
		return wrap(fmt::format("HTTP/1.1 200 OK\r\n"
		                        "Content-Type: {}\r\n"
		                        "Content-Length: {}\r\n"
		                        "Cache-Control: no-cache\r\n"
		                        "\r\n"
		                        "{}",
		                        response->content_type, response->contents.size(), response->contents),
		            size);
	}
}  // namespace quick_dra::gui
