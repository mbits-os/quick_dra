// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include "curl.h"

template <typename T>
struct prop_type {
	using type = T;
	using result = T;
};

template <>
struct prop_type<char const*> {
	using type = std::string;
	using result = std::string const&;
};

class CURL {
public:
	virtual ~CURL();

#define X(NAME, TYPE)                                               \
	virtual CURLcode setopt(CURL_##NAME##_option key, TYPE value) { \
		NAME##_props_[key] = value;                                 \
		return CURLE_OK;                                            \
	}                                                               \
	prop_type<TYPE>::type const* getopt(CURL_##NAME##_option key) { \
		auto it = NAME##_props_.find(key);                          \
		if (it == NAME##_props_.end()) {                            \
			return nullptr;                                         \
		}                                                           \
		return &it->second;                                         \
	}
	OPTION(X)
#undef X

#define X(NAME, TYPE)                                               \
	virtual CURLcode setinfo(CURL_##NAME##_info key, TYPE value) {  \
		NAME##_info_[key] = value;                                  \
		return CURLE_OK;                                            \
	}                                                               \
	virtual CURLcode getinfo(CURL_##NAME##_info key, TYPE* value) { \
		auto it = NAME##_info_.find(key);                           \
		if (it == NAME##_info_.end()) {                             \
			return CURL_LAST;                                       \
		}                                                           \
		*value = it->second;                                        \
		return CURLE_OK;                                            \
	}
	INFO(X)
#undef X

	virtual CURLcode perform() = 0;
	virtual CURLHcode header(const char* name,
	                         size_t index,
	                         curl_header** hout);

	void set_response(
	    long status,
	    std::string_view contents,
	    std::map<std::string_view, std::string_view> const& headers = {});

	template <std::derived_from<CURL> Impl>
	static std::unique_ptr<CURL> create() {
		return std::make_unique<Impl>();
	}

private:
#define X(NAME, TYPE) \
	std::map<CURL_##NAME##_option, prop_type<TYPE>::type> NAME##_props_;
	OPTION(X)
#undef X
#define X(NAME, TYPE) \
	std::map<CURL_##NAME##_info, prop_type<TYPE>::type> NAME##_info_;
	INFO(X)
#undef X

	struct header_storage {
		std::string name;
		std::string value;
		curl_header result{};
	};
	std::map<std::string, std::unique_ptr<header_storage>> headers_{};
};

using mock_curl_factory = std::unique_ptr<CURL> (*)(void* ptr);

std::pair<mock_curl_factory, void*> set_curl_factory(mock_curl_factory,
                                                     CURLcode = CURLE_OK,
                                                     void* ptr = nullptr);
template <typename Lambda>
    requires requires(Lambda const& lambda) {
	    { lambda() } -> std::convertible_to<std::unique_ptr<CURL>>;
    }
std::pair<mock_curl_factory, void*> set_curl_factory(Lambda const& lambda,
                                                     CURLcode init = CURLE_OK) {
	auto const wrapper = +[](void* ptr) {
		auto const cb = reinterpret_cast<Lambda const*>(ptr);
		auto result = (*cb)();
		return std::unique_ptr<CURL>{std::move(result)};
	};

	return set_curl_factory(wrapper, init, &lambda);
}