// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <stdlib.h>
#include <cstdlib>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/base/str.hpp>

#ifndef _WIN32
#define _putenv(E) putenv(E)
#endif

namespace quick_dra::testing {
	struct env_save {
		std::optional<std::string> value{};
		env_save(char const* name) {
			auto const c_value = std::getenv(name);
			if (c_value) value = fmt::format("{}={}", name, c_value);
		};
		~env_save() {
			if (value) {
				_putenv(value->data());
			}
		}
	};

	TEST(paths, home) {
		env_save orig_HOME{"HOME"};
		char HOME[] = "HOME=some/value";
		_putenv(HOME);
		auto const argument = std::filesystem::path{"overriden"} / "directory" /
		                      ".quick_dra.yaml"sv;

		ASSERT_EQ(platform::get_config_path(std::nullopt),
		          std::filesystem::path{"some/value"} / ".quick_dra.yaml"sv);

		ASSERT_EQ(platform::get_config_path(as_str(argument.u8string())),
		          argument);
	}
}  // namespace quick_dra::testing