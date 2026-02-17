// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "mock.hpp"

namespace {
	std::pair<mock_curl_factory, void*> global_factory{nullptr, nullptr};

	auto global_init_value = CURLE_OK;
}  // namespace

void set_curl_init_result(CURLcode value) { global_init_value = value; }
std::pair<mock_curl_factory, void*> set_curl_factory(mock_curl_factory factory,
                                                     CURLcode init_value,
                                                     void* ptr) {
	auto tmp = global_factory;
	global_factory = {factory, ptr};
	global_init_value = init_value;
	return tmp;
}

CURLcode curl_global_init(global_default_value) { return global_init_value; }
void curl_global_cleanup() {}

CURL* curl_easy_init() {
	if (!global_factory.first) {
		return nullptr;
	}
	auto result = (*global_factory.first)(global_factory.second);
	return result.release();
}

void curl_easy_cleanup(CURL* impl) { delete impl; }

#define X(NAME, TYPE)                                              \
	CURLcode curl_easy_setopt(CURL* ptr, CURL_##NAME##_option key, \
	                          TYPE value) {                        \
		return ptr->setopt(key, value);                            \
	}
OPTION(X)
#undef X

#define X(NAME, TYPE)                                                          \
	CURLcode curl_easy_getinfo(CURL* ptr, CURL_##NAME##_info key, TYPE* dst) { \
		return ptr->getinfo(key, dst);                                         \
	}
INFO(X)
#undef X

CURLcode curl_easy_perform(CURL* ptr) { return ptr->perform(); }

CURLHcode curl_easy_header(CURL* easy,
                           const char* name,
                           size_t index,
                           CURLH_HEADER_t,
                           int,
                           curl_header** hout) {
	return easy->header(name, index, hout);
}

CURL::~CURL() = default;

CURLHcode CURL::header(const char* name, size_t index, curl_header** hout) {
	if (!hout || !name || !*name) {
		return CURLHE_BAD_ARGUMENT;
	}

	std::string key{name};
	for (auto& c : key) {
		c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	}

	auto it = headers_.find(key);
	if (it == headers_.end()) {
		return CURLHE_MISSING;
	}

	auto& stg = *it->second;
	stg.result.name = stg.name.data();
	stg.result.value = stg.value.data();
	*hout = &stg.result;
	return CURLHE_OK;
}

void CURL::set_response(
    long status,
    std::string_view contents,
    std::map<std::string_view, std::string_view> const& headers) {
	auto const pp_cb = getopt(CURLOPT_WRITEFUNCTION);
	if (pp_cb) {
		auto const p_user = getopt(CURLOPT_WRITEDATA);
		auto const user = p_user ? *p_user : nullptr;
		auto content_bytes =
		    std::vector<char>{contents.begin(), contents.end()};
		(**pp_cb)(content_bytes.data(), content_bytes.size(), 1, user);
	}

	setinfo(CURLINFO_RESPONSE_CODE, status);

	headers_.clear();
	for (auto const& [key_view, value_view] : headers) {
		std::string key;
		key = key_view;
		for (auto& c : key) {
			c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
		}

		auto it = headers_.lower_bound(key);
		if (it == headers_.end() || it->first != key) {
			it = headers_.insert(
			    it, {std::move(key), std::make_unique<header_storage>()});
			auto& stg = it->second;
			stg->name = key_view;
			stg->result = {.name{nullptr},
			               .value{nullptr},
			               .amount{1},
			               .index{0},
			               .origin{},
			               .anchor{}};
		}
		auto& storage = it->second;
		storage->value = value_view;
	}
}