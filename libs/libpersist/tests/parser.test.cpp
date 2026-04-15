// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/std.h>
#include <gtest/gtest.h>
#include <array>
#include <filesystem>
#include <span>
#include <utility>
#include <vector>
#include <yaml/parser.hpp>

namespace yaml::testing {
	using std::literals::operator""s;
	using std::literals::operator""sv;

	template <typename T>
	struct test_struct {
		T child{};
		using value_type = T;

		auto operator<=>(test_struct const&) const noexcept = default;
		bool read(yaml::ref_ctx const& ref) {
			if (!read_key(ref, "child", child)) return ref.error("while reading `child`");
			return true;
		}
	};

}  // namespace yaml::testing

namespace fmt {
	template <typename T>
	struct formatter<yaml::testing::test_struct<T>> : formatter<std::string> {
		template <typename FormatContext>
		auto format(yaml::testing::test_struct<T> const& value, FormatContext& ctx) const {
			return formatter<std::string>::format(fmt::format("{{.child={}}}", value.child), ctx);
		}
	};
}  // namespace fmt

namespace yaml::testing {
	template <typename T>
	void PrintTo(test_struct<T> const& val, std::ostream* os) {
		*os << fmt::to_string(val);
	}

	template <typename Payload, typename StringLike>
	struct parsed_result {
		Payload value;
		StringLike log;
	};

	template <typename Payload>
	parsed_result<std::optional<Payload>, std::string> parse_yaml(std::string const& yaml_text) {
		::testing::internal::CaptureStderr();
		auto result = parsed_result<std::optional<Payload>, std::string>{
		    .value = parser::parse_yaml_text<Payload>(yaml_text, "input"s),
		    .log{},
		};
		result.log = ::testing::internal::GetCapturedStderr();

		return result;
	}

	template <typename Payload>
	parsed_result<std::optional<Payload>, std::string> parse_yaml_file(std::filesystem::path const& path) {
		::testing::internal::CaptureStderr();
		auto result = parsed_result<std::optional<Payload>, std::string>{
		    .value = parser::parse_yaml_file<Payload>(path, []() { fmt::print(stderr, "example file not found\n"); }),
		    .log{},
		};
		result.log = ::testing::internal::GetCapturedStderr();

		return result;
	}

	template <typename T>
	struct wrap_t {
		T const* ref;
		wrap_t(T const& ref) : ref{&ref} {}
		friend std::ostream& operator<<(std::ostream& out, wrap_t const& w) { return out << w.ref; }
	};

	template <>
	struct wrap_t<std::monostate> {
		wrap_t(std::monostate) {}
		friend std::ostream& operator<<(std::ostream& out, wrap_t const&) { return out; }
	};

	template <typename Payload, typename Arg = std::monostate>
	void _test_payload(parsed_result<std::optional<Payload>, std::string> const& actual,
	                   parsed_result<std::optional<Payload>, std::string_view> const& expected,
	                   Arg const& arg = {}) {
		ASSERT_EQ(actual.value, expected.value) << wrap_t{arg};
		ASSERT_EQ(actual.log, expected.log) << wrap_t{arg};
	}

	template <typename Payload>
	bool test_payload(parsed_result<std::optional<Payload>, std::string> const& actual,
	                  parsed_result<std::optional<Payload>, std::string_view> const& expected) {
		_test_payload<Payload>(actual, expected);
		return !::testing::Test::HasFailure();
	}

	template <typename Payload>
	bool test_payload(std::string_view yaml_text, std::string_view log, std::optional<Payload> const& exp = {}) {
		_test_payload<Payload>(parse_yaml<Payload>({
		                           yaml_text.data(),
		                           yaml_text.size(),
		                       }),
		                       {exp, log});
		return !::testing::Test::HasFailure();
	}

	template <typename Payload>
	bool test_payload_file(std::filesystem::path const& path,
	                       std::string_view log,
	                       std::optional<Payload> const& exp = {}) {
		_test_payload<Payload>(parse_yaml_file<Payload>(path), {exp, log}, path);
		return !::testing::Test::HasFailure();
	}

	TEST(yaml, invalid_text) {
		test_payload<std::map<std::string, std::string>>(
		    "key: value\n"
		    "non-key\n"
		    ""sv,
		    "input:4:2: error: ERROR: could not find ':' colon after key\n"
		    "\n"
		    "\n"sv);
	}

	TEST(yaml, read_value_array) {
		test_payload<std::vector<std::string>>("child: value"sv, "input:1:1: error: expecting an array\n"sv);
	}

	TEST(yaml, read_value_map) {
		test_payload<std::map<std::string, std::string>>(
		    "- one\n"
		    "- two\n"
		    "- three\n"sv,
		    "input:1:1: error: expecting a map\n"sv);
	}

	TEST(yaml, read_value_variant) {
		test_payload<std::variant<bool, unsigned>>("'key'\n"sv,
		                                           "input:1:2: error: expecting a true or false\n"
		                                           "input:1:2: error: expecting a positive number\n"
		                                           "input:1:2: error: cannot match any union alternative\n"sv);
	}

	TEST(yaml, read_value_number) {
		test_payload<unsigned>(
		    "- one\n"
		    "- two\n"sv,
		    ""sv);
	}

	TEST(yaml, read_value_bool) {
		test_payload<bool>(
		    "- one\n"
		    "- two\n"sv,
		    ""sv);
		test_payload<bool>("True\n"sv, ""sv, true);
		test_payload<bool>("FALSE\n"sv, ""sv, false);
		test_payload<bool>("''\n"sv, ""sv, false);
		test_payload<bool>("truthy\n"sv, "input:1:1: error: expecting a true or false\n"sv);
	}

	TEST(yaml, read_value_string) {
		test_payload<std::string>(
		    "- one\n"
		    "- two\n"sv,
		    ""sv);
		test_payload<std::string>("''\n"sv, ""sv, ""s);
	}

	TEST(yaml, read_key_missing) {
		test_payload<test_struct<std::string>>("not-child: ''"sv,
		                                       "input:1:1: error: expecting `child`\n"
		                                       "input:1:1: error: while reading `child`\n"sv);
	}

	TEST(yaml, read_key_not_map) {
		test_payload<test_struct<test_struct<std::string>>>("child:"sv,
		                                                    "input:1:1: error: expecting `child`\n"
		                                                    "input:1:1: error: while reading `child`\n"
		                                                    "input:1:1: error: while reading `child`\n"sv);
	}

	TEST(yaml, read_key_not_map_optional_target) {
		test_payload<test_struct<test_struct<std::optional<std::string>>>>(
		    "child:"sv,
		    "input:1:1: error: while reading `child`\n"
		    "input:1:1: error: while reading `child`\n"sv);
	}

	TEST(yaml, read_key_number_no_value) {
		test_payload<test_struct<unsigned>>("child:"sv, ""sv, test_struct{.child = 0u});
	}

	TEST(yaml, read_file) {
		auto const path = std::filesystem::current_path() / "data"sv / "test"sv / "parser.test.yaml"sv;
		using strings = test_struct<std::vector<std::string>>;
		test_payload_file(path, ""sv, std::optional{strings{.child = strings::value_type{"one"s, "two"s, "three"s}}});
	}

	TEST(yaml, read_nonexistent_file) {
		auto const path = std::filesystem::current_path() / "data"sv / "test"sv / "parser.test.yml"sv;
		using strings = test_struct<std::vector<std::string>>;
		test_payload_file<strings>(path, "example file not found\n"sv);
	}
}  // namespace yaml::testing
