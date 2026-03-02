// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <stdlib.h>
#include <cstdlib>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/base/str.hpp>

#ifdef _WIN32
static int setenv(const char* name, const char* value, int) {
	return _putenv_s(name, value);
}
#endif

namespace quick_dra::testing {
	struct env_swap {
		char const* name;
		std::optional<std::string> value{};
		env_swap(char const* name, const char* next) : name{name} {
			auto const c_value = std::getenv(name);
			if (c_value) value = c_value;
			setenv(name, next, 1);
		};
		env_swap(env_swap const&) = delete;
		env_swap(env_swap&&) = delete;
		~env_swap() {
			if (value) {
				setenv(name, value->data(), 1);
			}
		}
	};

	TEST(paths, home) {
		env_swap HOME{"HOME", "some/value"};
		auto const argument = std::filesystem::path{"overriden"} / "directory" /
		                      ".quick_dra.yaml"sv;

		ASSERT_EQ(platform::get_config_path(std::nullopt),
		          std::filesystem::path{"some/value"} / ".quick_dra.yaml"sv);

		ASSERT_EQ(platform::get_config_path(as_str(argument.u8string())),
		          argument);
	}
}  // namespace quick_dra::testing