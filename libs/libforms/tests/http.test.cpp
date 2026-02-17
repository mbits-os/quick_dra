// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <curl/mock.hpp>
#include <quick_dra/io/http.hpp>
#include <span>

namespace std {
	template <typename T1, typename T2>
	bool operator==(std::span<T1> const& lhs, std::span<T2> const& rhs) {
		if (lhs.size_bytes() != rhs.size_bytes()) {
			return false;
		}

		return std::memcmp(lhs.data(), rhs.data(), lhs.size_bytes()) == 0;
	}
};  // namespace std

namespace quick_dra::testing {
	using std::literals::operator""s;
	using std::literals::operator""sv;

	static constexpr auto dummy_content =
	    "content content content content content content content "
	    "content content content content content content content "
	    "content content content content content content content "
	    "content content content content content content content "
	    "content content content content content content content "sv;

	void assert_response_empty(http_response const& actual) {
		ASSERT_EQ(actual.status, 0);
		ASSERT_TRUE(actual.content_type.type.empty());
		ASSERT_TRUE(actual.charset.empty());
		ASSERT_TRUE(actual.content.empty());
	}

#define ASSERT_RESPONSE_EMPTY(value)               \
	do {                                           \
		assert_response_empty(actual);             \
		if (::testing::Test::HasFailure()) return; \
	} while (false)

	TEST(http, curl_bad_init) {
		set_curl_factory(nullptr, CURLE_FAILED_INIT);
		::testing::internal::CaptureStderr();
		auto const actual = http_get("http://example.com/"s);
		auto const log = ::testing::internal::GetCapturedStderr();
		ASSERT_RESPONSE_EMPTY(actual);
		ASSERT_EQ(log,
		          "Quick-DRA: error: CURL global initialization failed\n"
		          "           while downloading http://example.com/\n"sv);
	}

	class ImplNoPerform : public CURL {
		CURLcode perform() override { return CURLE_AUTH_ERROR; }
	};

	TEST(http, curl_bad_perform) {
		set_curl_factory<ImplNoPerform>();
		::testing::internal::CaptureStderr();
		auto const actual = http_get("http://example.com/"s);
		auto const log = ::testing::internal::GetCapturedStderr();
		ASSERT_RESPONSE_EMPTY(actual);
		ASSERT_TRUE(log.empty());
	}

	class ImplNoResponseCode : public CURL {
		CURLcode perform() override { return CURLE_OK; }
	};

	TEST(http, curl_bad_status) {
		set_curl_factory<ImplNoResponseCode>();
		::testing::internal::CaptureStderr();
		auto const actual = http_get("http://example.com/"s);
		auto const log = ::testing::internal::GetCapturedStderr();
		ASSERT_RESPONSE_EMPTY(actual);
		ASSERT_TRUE(log.empty());
	}

	class ImplNoContentType : public CURL {
		CURLcode perform() override {
			set_response(200, dummy_content);
			return CURLE_OK;
		}
	};

	TEST(http, curl_content_type_missing) {
		set_curl_factory<ImplNoContentType>();
		::testing::internal::CaptureStderr();
		auto const actual = http_get("http://example.com/"s);
		auto const log = ::testing::internal::GetCapturedStderr();
		ASSERT_RESPONSE_EMPTY(actual);
		ASSERT_TRUE(log.empty());
	}

	class Impl : public CURL {
		CURLcode perform() final {
			set_response(200, dummy_content,
			             {{"Content-Type"sv, getContentType()}});
			return CURLE_OK;
		}

		virtual std::string_view getContentType() const = 0;
	};

	class ImplNoCharset : public Impl {
		std::string_view getContentType() const override {
			return "text/dummy; token-a; token-b; name=value"sv;
		};
	};

	class ImplCharset : public Impl {
		std::string_view getContentType() const override {
			return "video/clip; charset=utf-future"sv;
		};
	};

	TEST(http, curl_content_no_charset) {
		set_curl_factory<ImplNoCharset>();
		::testing::internal::CaptureStderr();
		auto const actual = http_get("http://example.com/"s);
		auto const log = ::testing::internal::GetCapturedStderr();
		ASSERT_TRUE(actual);
		ASSERT_EQ(std::span{actual.content}, std::span{dummy_content});
		ASSERT_EQ(actual.content_type.type, "text"sv);
		ASSERT_EQ(actual.content_type.subtype, "dummy"sv);
		ASSERT_TRUE(actual.charset.empty());
		ASSERT_TRUE(log.empty());
	}

	TEST(http, curl_good_content) {
		set_curl_factory<ImplCharset>();
		::testing::internal::CaptureStderr();
		auto const actual = http_get("http://example.com/"s);
		auto const log = ::testing::internal::GetCapturedStderr();
		ASSERT_TRUE(actual);
		ASSERT_EQ(std::span{actual.content}, std::span{dummy_content});
		ASSERT_EQ(actual.content_type.type, "video"sv);
		ASSERT_EQ(actual.content_type.subtype, "clip"sv);
		ASSERT_EQ(actual.charset, "utf-future"sv);
		ASSERT_TRUE(log.empty());
	}
}  // namespace quick_dra::testing
