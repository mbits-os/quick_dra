// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#define NOMINMAX

#include <Windows.h>
#include <filesystem>

namespace quick_dra::platform {
	std::filesystem::path _exec_path() {
		wchar_t modpath[2048];
		GetModuleFileNameW(nullptr, modpath,
		                   sizeof(modpath) / sizeof(modpath[0]));
		return modpath;
	}
}  // namespace quick_dra::platform
