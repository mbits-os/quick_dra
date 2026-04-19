// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <cstdlib>

static inline void* webui_malloc(size_t size) { return std::malloc(size + 1); }
static inline void webui_free(void* ptr) { return std::free(ptr); }
static inline const char* webui_get_mime_type(const char*) { return "test/data"; }