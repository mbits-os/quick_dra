// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#define NOMINMAX

#include <curl/curl.h>
#include <fmt/format.h>
#include <memory>
#include <quick_dra/io/http.hpp>
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
	}  // namespace

	bool download_file(std::string const& url, std::string& out) try {
		out.clear();

		auto curl = curl_easy_init();
		if (!curl) return false;
		std::string version;
		curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, "curl");
		curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
		curl_write_callback write_cb = [](char* contents, std::size_t size,
		                                  std::size_t nmemb,
		                                  void* userp) -> std::size_t {
			auto& str = *static_cast<std::string*>(userp);
			auto realsize = size * nmemb;
			str.append(contents, realsize);
			return realsize;
		};
		curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, write_cb);
		curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &out);
		curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, false);
		auto res = curl_easy_perform(curl.get());
		return (res == CURLE_OK);

	} catch (std::exception& error) {
		fmt::print(stderr,
		           "quick_dra: error: {}\n"
		           "           while downloading {}\n",
		           error.what(), url);
		return false;
	}
}  // namespace quick_dra
