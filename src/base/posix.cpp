// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>
#include <filesystem>

namespace quick_dra::platform {
	std::filesystem::path _exec_path() {
		using namespace std::literals;
		std::error_code ec;
		static constexpr std::string_view self_links[] = {
		    "/proc/self/exe"sv,
		    "/proc/curproc/file"sv,
		    "/proc/curproc/exe"sv,
		    "/proc/self/path/a.out"sv,
		};
		for (auto path : self_links) {
			auto link = std::filesystem::read_symlink(path, ec);
			if (!ec) return link;
		}
		[[unlikely]];  // GCOV_EXCL_LINE[POSIX]
		return {};     // GCOV_EXCL_LINE[POSIX]
	}
}  // namespace quick_dra::platform
