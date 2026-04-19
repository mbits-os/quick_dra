// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <memory>
#include <optional>
#include <quick_dra/gui/webui.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace quick_dra::gui {
	struct html_response {
		std::string_view contents{};
		std::string_view content_type{};
		std::optional<std::string> redirect{};
	};

	webui::ptr<char> wrap_response(std::optional<html_response> const& response,
	                               std::string_view tag,
	                               std::string_view path,
	                               int& size);

	template <typename Final>
	class basic_filesystem {
	public:
		webui::ptr<char> http_response(std::string_view path, int& size) const {
			auto const impl = static_cast<Final const*>(this);
			std::vector<char> stg{};
			return wrap_response(impl->respond(path, stg), Final::tag(), path, size);
		}

		static const void* global_handler(const char* path, int* length) {
			int size{};
			auto payload = Final::get_global().http_response(path, size);
			if (payload && length) {
				*length = size;
			}
			return payload.release();
		}
	};
}  // namespace quick_dra::gui
