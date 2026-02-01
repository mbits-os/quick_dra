// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#if defined __cpp_nontype_template_parameter_class
#define CNTTP_COMPILER_CHECK 1
#elif defined __cpp_nontype_template_args
// compiler which defines correctly feature test macro (not you clang)
#if __cpp_nontype_template_args >= 201911L
#define CNTTP_COMPILER_CHECK 1
#elif __cpp_nontype_template_args >= 201411L
// appleclang 13+
#if defined __apple_build_version__
#if defined __clang_major__ && __clang_major__ >= 13
// but only in c++20 and more
#if __cplusplus > 201703L
#define CNTTP_COMPILER_CHECK 1
#endif
#endif
#else
// clang 12+
#if defined __clang_major__ && __clang_major__ >= 12
// but only in c++20 and more
#if __cplusplus > 201703L
#define CNTTP_COMPILER_CHECK 1
#endif
#endif
#endif
#endif
#endif

#ifndef CNTTP_COMPILER_CHECK
#define CNTTP_COMPILER_CHECK 0
#endif
