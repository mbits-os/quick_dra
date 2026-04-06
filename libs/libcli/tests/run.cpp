// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "run.hpp"
#include <args/sys.hpp>
#include <ctre.hpp>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/base/str.hpp>
#include <quick_dra/version.hpp>

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

int tool(args::args_view const& arguments);

namespace quick_dra::builtin::testing {
	namespace {

#ifdef WIN32
		char* mkdtemp(char* buffer) {
			_mktemp(buffer);
			auto const path = std::filesystem::path{as_u8v(buffer)};
			create_directories(path);
			return buffer;
		}
		struct win32_utf8 {
			win32_utf8() { SetConsoleOutputCP(CP_UTF8); }
		};

		win32_utf8 _utf8{};
#endif

		struct temp_directory {
			temp_directory() { std::filesystem::current_path(dirname_); }
			temp_directory(temp_directory const&) = delete;
			temp_directory(temp_directory&&) = delete;
			~temp_directory() {
				std::error_code ec{};
				std::filesystem::current_path(cwd_);
				std::filesystem::remove_all(dirname_, ec);
			}

			[[nodiscard]] std::filesystem::path const& cwd() const noexcept {
				return dirname_;
			};

		private:
			std::filesystem::path make_temp_dir() {
				auto const path = std::filesystem::temp_directory_path() /
				                  fmt::format("dirXXXXXX");
				auto name = as_str(path.u8string());
				return as_u8v(mkdtemp(name.data()));
			}

			std::filesystem::path dirname_{make_temp_dir()};
			std::filesystem::path cwd_{std::filesystem::current_path()};
		};

		void skip_ws(std::string_view::iterator& it,
		             std::string_view::iterator const& end) {
			while (it != end && std::isspace(static_cast<unsigned char>(*it)))
				++it;
		}

		std::string quoted(std::string_view::iterator& it,
		                   std::string_view::iterator const& end,
		                   char quote) {
			std::string result{};
			auto start = it;
			while (it != end && *it != quote)
				++it;

			result.insert(result.end(), start, it);

			if (it != end) ++it;
			return result;
		}

		std::string escaped(std::string_view::iterator& it,
		                    std::string_view::iterator const& end) {
			std::string result{};
			auto escaped{false};
			while (it != end) {
				auto const c = *it;
				if (escaped) {
					result.push_back(c);
					++it;
					escaped = false;
					continue;
				}

				if (c == '\\') {
					escaped = true;
					++it;
					continue;
				}

				if (c == '\'' || c == '"' ||
				    std::isspace(static_cast<unsigned char>(c)))
					break;

				result.push_back(c);
				++it;
			}

			return result;
		}

		std::string get_argument(std::string_view::iterator& it,
		                         std::string_view::iterator const& end) {
			std::string result;
			while (it != end &&
			       !std::isspace(static_cast<unsigned char>(*it))) {
				if (*it == '\'') {
					++it;
					result.append(quoted(it, end, '\''));
					continue;
				}
				if (*it == '"') {
					++it;
					result.append(quoted(it, end, '"'));
					continue;
				}
				result.append(escaped(it, end));
			}
			skip_ws(it, end);
			return result;
		}

		std::vector<std::string> split_args(std::string_view args) {
			std::vector<std::string> result{};
			auto it = args.begin();
			auto end = args.end();
			skip_ws(it, end);
			while (it != end) {
				result.emplace_back(get_argument(it, end));
			}
			return result;
		}

		struct closed_run {
			int returncode{0};
			std::string stdout{};
			std::string stderr{};

			void append(closed_run const& post) {
				returncode = post.returncode;
				stdout += post.stdout;
				stderr += post.stderr;
			}
		};

		struct exit_code {
			int value;
		};

		[[noreturn]] void throw_on_exit(int code) {
			throw exit_code{.value = code};
		}

		closed_run run(std::string_view arguments) {
			auto args = split_args(arguments);
			std::vector<char*> argv{};
			argv.reserve(args.size() + 1);
			std::transform(args.begin(), args.end(), std::back_inserter(argv),
			               [](auto& s) { return s.data(); });
			argv.push_back(nullptr);

			closed_run result{};

			::testing::internal::CaptureStdout();
			::testing::internal::CaptureStderr();

			args::set_exit(throw_on_exit);
			try {
				result.returncode = tool(
				    {"qdra"sv, args::arglist{static_cast<unsigned>(args.size()),
				                             argv.data()}});
			} catch (exit_code const& ec) {
				result.returncode = ec.value;
			}

			result.stdout = ::testing::internal::GetCapturedStdout();
			result.stderr = ::testing::internal::GetCapturedStderr();

			return result;
		}

		inline std::filesystem::path _data_dir() {
			// reverse of build/<config>/bin
			auto const root =
			    platform::exec_dir().parent_path().parent_path().parent_path();
			return root / "libs"sv / "cli"sv / "tests"sv / "data"sv;
		}

		inline std::filesystem::path const& data_dir() {
			static auto const path = _data_dir();
			return path;
		}

		std::string file_contents(std::filesystem::path const& path) {
			auto file = std::ifstream{path};
			auto str = std::ostringstream{};
			str << file.rdbuf();
			return std::move(str).str();
		}

		std::string patched(std::string_view content) {
			return join(split_sv(content, sep_view_t{version::string}),
			            "$VERSION"_sep);  //-V601
		}

		void expect_output(std::string_view actual,
		                   std::string_view expected,
		                   compare cmp) {
			switch (cmp) {
				case compare::all:
					EXPECT_EQ(actual, expected);
					return;
				case compare::begin:
					if (actual.length() > expected.length()) {
						EXPECT_EQ(actual.substr(0, expected.length()),
						          expected);
					} else {
						EXPECT_EQ(actual, expected);
					}
					return;
				case compare::end:
					if (actual.length() > expected.length()) {
						EXPECT_EQ(
						    actual.substr(actual.length() - expected.length()),
						    expected);
					} else {
						EXPECT_EQ(actual, expected);
					}
					return;
			}
		}

		void expect_test(runnable_testcase const& expected) {
			temp_directory tmp_dir{};  //-V821

			if (!expected.config_name.empty() || !expected.config.empty()) {
				auto path = expected.config_name;
				if (path.empty()) path = ".quick_dra.yaml"sv;

				auto const output = tmp_dir.cwd() / path;
				{
					std::ofstream out{output};
					out << expected.config;
				}

				if (expected.mode != perms::none) {
					std::filesystem::permissions(
					    output, expected.mode,
					    std::filesystem::perm_options::replace);
				}
			}

			auto actual = run(expected.args);
			for (auto const& post : expected.post) {
				if (actual.returncode) break;
				actual.append(run(post));
			}

			std::string_view expected_stdout{};
			std::string_view expected_stderr{};

			std::string lazy_stdout{};  //-V821
			std::string lazy_stderr{};  //-V821

			if (std::holds_alternative<std::string_view>(expected.stdout)) {
				expected_stdout = std::get<std::string_view>(expected.stdout);
			} else {
				auto const fn = std::get<std::string (*)()>(expected.stdout);
				lazy_stdout = fn();
				expected_stdout = lazy_stdout;
			}

			if (std::holds_alternative<std::string_view>(expected.stderr)) {
				expected_stderr = std::get<std::string_view>(expected.stderr);
			} else {
				auto const fn = std::get<std::string (*)()>(expected.stderr);
				lazy_stderr = fn();
				expected_stderr = lazy_stderr;
			}

			EXPECT_EQ(actual.returncode, expected.returncode);
			expect_output(actual.stdout, expected_stdout,
			              expected.check_stdout);
			expect_output(actual.stderr, expected_stderr,
			              expected.check_stderr);

			if (expected.writes) {
				auto const& file = *expected.writes;

				if (file.cmp.empty()) {
					std::error_code ec{};
					EXPECT_TRUE(std::filesystem::is_regular_file(file.name, ec))
					    << "Wrote: " << file.name;
					EXPECT_TRUE(ec) << "Wrote: " << file.name;
				} else {
					auto const cmp_path = data_dir() / file.cmp;
					auto const actual_content =
					    patched(file_contents(file.name));
					std::error_code ec{};
					if (!std::filesystem::exists(cmp_path, ec)) {
						std::ofstream out{cmp_path,
						                  std::ios::out | std::ios::binary};
						out.write(actual_content.data(),
						          static_cast<std::streamsize>(
						              actual_content.size()));
					} else {
						auto const expected_content =
						    file_contents(data_dir() / file.cmp);
						EXPECT_EQ(actual_content, expected_content)
						    << fmt::to_string(data_dir() / file.cmp);
					};
				}
			}
		}
	}  // namespace

	bool runnable_testcase::run_test() const {
		testing::expect_test(*this);
		if (::testing::Test::HasFailure()) {
			fmt::print("Args: {}\n", args);
			if (!post.empty()) {
				fmt::print("Post:\n", args);
				for (auto const& cmd : post) {
					fmt::print("- {}\n", cmd);
				}
			}
		}
		return !::testing::Test::HasFatalFailure();
	}

	TEST_P(cli_test, run_cli) {
		auto const& param = GetParam();
		param.run_test();
	}
}  // namespace quick_dra::builtin::testing
