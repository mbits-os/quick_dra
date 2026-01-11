// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#define NOMINMAX

#include <curl/curl.h>
#include <fmt/format.h>
#include <memory>
#include <quick_dra/base/str.hpp>
#include <quick_dra/io/http.hpp>
#include <quick_dra/version.hpp>
#include <span>
#include <stdexcept>

namespace quick_dra {
	namespace {
		struct curl_global_initer {
			curl_global_initer() {
				if (::curl_global_init(CURL_GLOBAL_DEFAULT) != 0)
					throw std::runtime_error(
					    "CURL global initialization failed");
			}
			~curl_global_initer() { ::curl_global_cleanup(); }
		};

		struct curl_cleanup {
			void operator()(CURL* p) const { ::curl_easy_cleanup(p); }
		};

		using curl_ptr = std::unique_ptr<CURL, curl_cleanup>;

		curl_ptr curl_easy_init() {
			static const curl_global_initer init{};
			return curl_ptr{::curl_easy_init()};
		}

		CURLcode curl_easy_setopt(curl_ptr const& curl,
		                          CURLoption option,
		                          auto arg) {
			return ::curl_easy_setopt(curl.get(), option, arg);
		}

		CURLcode curl_easy_perform(curl_ptr const& curl) {
			return ::curl_easy_perform(curl.get());
		}

		auto ua_version() noexcept {
			if constexpr (version::major < 1) {
				return version::string;
			} else {
				return version::string_short;
			}
		}
	}  // namespace

	http_response http_get(std::string const& url) try {
		http_response response{};

		auto curl = curl_easy_init();
		if (!curl) return response;

		auto const ua = fmt::format("{}/{}", version::program, ua_version());

		curl_easy_setopt(curl, CURLOPT_USERAGENT, ua.c_str());
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_write_callback write_cb = [](char* contents, std::size_t size,
		                                  std::size_t nmemb,
		                                  void* userp) -> std::size_t {
			auto& resp = *static_cast<http_response*>(userp);
			auto realsize = size * nmemb;
			auto bytes = reinterpret_cast<std::byte*>(contents);
			resp.content.insert(resp.content.end(), bytes, bytes + realsize);
			return realsize;
		};
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
		if (curl_easy_perform(curl) != CURLE_OK) {
			return response.cleaned();
		}

		long code;
		if (curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &code) !=
		    CURLE_OK) {
			return response.cleaned();
		}
		response.status = code;

		curl_header* header{};
		if (curl_easy_header(curl.get(), "Content-Type", 0, CURLH_HEADER, -1,
		                     &header) != CURLHE_OK) {
			return response.cleaned();
		}
		auto const view = std::string_view{header->value};
		auto const split_view = split_sv(';', view);

		auto const type_subtype = split_sv('/', split_view[0], 1);
		if (type_subtype.size() == 2) {
			response.content_type.type = strip_sv(type_subtype[0]);
			response.content_type.subtype = strip_sv(type_subtype[1]);
		}

		auto const params = std::span{split_view}.subspan(1);
		for (auto const& param_view : params) {
			auto const name_value = split_sv('=', param_view, 1);
			if (name_value.size() != 2) {
				continue;
			}
			if (strip_sv(name_value[0]) != "charset"sv) {
				continue;
			}
			response.charset = strip_sv(name_value[1]);
		}

		return response;
	} catch (std::exception& error) {
		fmt::print(stderr,
		           "Quick-DRA: error: {}\n"
		           "           while downloading {}\n",
		           error.what(), url);
		return http_response{};
	}
}  // namespace quick_dra
