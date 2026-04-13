// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

#include <fmt/format.h>
#include <quick_dra/conv/args_parser.hpp>

extern int gui_tool(::args::args_view const&);

// GCOV_EXCL_START
#ifdef _WIN32
static std::string ut8f_str(wchar_t const* arg) {
	if (!arg) return {};

	auto size = WideCharToMultiByte(CP_UTF8, 0, arg, -1, nullptr, 0, nullptr, nullptr);
	auto out = std::make_unique<char[]>(size + 1);
	WideCharToMultiByte(CP_UTF8, 0, arg, -1, out.get(), size + 1, nullptr, nullptr);
	return out.get();
}

template <typename T>
struct local_free {
	void operator()(T* ptr) { LocalFree(ptr); }
};

template <typename T>
using local_ptr = std::unique_ptr<T, local_free<T>>;

static std::vector<std::string> wide_char_to_utf8() {
	std::vector<std::string> result{};
	int argc = 0;
	auto const argv_ptr = local_ptr<LPWSTR>{CommandLineToArgvW(GetCommandLineW(), &argc)};
	auto argv = argv_ptr.get();

	result.resize(argc);
	std::transform(argv, argv + argc, result.begin(), ut8f_str);
	return result;
}

int WINAPI wWinMain([[maybe_unused]] HINSTANCE hInstance,
                    [[maybe_unused]] HINSTANCE hPrevInstance,
                    [[maybe_unused]] PWSTR pCmdLine,
                    [[maybe_unused]] int nCmdShow) {
	int argc = 0;
	auto const argv_ptr = local_ptr<LPWSTR>{CommandLineToArgvW(GetCommandLineW(), &argc)};
	auto argv = argv_ptr.get();

	std::vector<std::string> utf8 = wide_char_to_utf8();
	utf8.resize(argc);
	std::transform(argv, argv + argc, utf8.begin(), ut8f_str);

	std::vector<char*> args{};
	args.resize(utf8.size() + 1);
	std::transform(utf8.begin(), utf8.end(), args.begin(), [](auto& s) { return s.data(); });
	args[argc] = nullptr;

	SetConsoleOutputCP(CP_UTF8);

	return gui_tool(::args::from_main(static_cast<int>(args.size() - 1), args.data()));
}

static std::vector<std::string> wide_char_to_utf8(int argc, wchar_t* argv[]) {
	std::vector<std::string> result{};
	result.resize(argc);
	std::transform(argv, argv + argc, result.begin(), ut8f_str);
	return result;
}

int wmain(int argc, wchar_t* argv[]) {
	auto utf8 = wide_char_to_utf8(argc, argv);
	std::vector<char*> args{};
	args.resize(utf8.size() + 1);
	std::transform(utf8.begin(), utf8.end(), args.begin(), [](auto& s) { return s.data(); });
	args[argc] = nullptr;

	SetConsoleOutputCP(CP_UTF8);

	return gui_tool(::args::from_main(static_cast<int>(args.size() - 1), args.data()));
}
#else
int main(int argc, char* argv[]) { return gui_tool(args::from_main(argc, argv)); }
#endif
// GCOV_EXCL_STOP
