// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <fmt/std.h>
#include <gtest/gtest.h>
#include <array>
#include <fstream>
#include <map>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/models/project_reader.hpp>
#include <ranges>
#include <span>
#include <utility>
#include <variant>
#include <vector>
#include <yaml/parser.hpp>

namespace quick_dra::testing {
	auto const& project_root() {
		// reverse of build/<config>/bin/tests
		static auto root = platform::exec_dir().parent_path().parent_path().parent_path().parent_path();
		return root;
	}
	auto const& test_data_dir() {
		static auto dir = project_root() / "libs"sv / "cli"sv / "tests"sv / "data"sv;
		return dir;
	}
	std::optional<std::string> get_file_contents(std::filesystem::path const& path) {
		std::optional<std::string> result{};
		std::ifstream in{path, std::ios::in | std::ios::binary};
		if (!in) {
			return result;
		}

		std::ostringstream contents;
		contents << in.rdbuf();
		result = std::move(contents).str();

		return result;
	}

	TEST(partial, file_not_found) {
		auto const filename = test_data_dir() / "no-such"sv;
		::testing::internal::CaptureStderr();
		partial::config::load_partial(filename, true);
		auto err = ::testing::internal::GetCapturedStderr();

		EXPECT_EQ(err, fmt::format("Quick-DRA: file {} will be created as needed.\n", filename));

		::testing::internal::CaptureStderr();
		partial::config::load_partial(filename, false);
		err = ::testing::internal::GetCapturedStderr();

		EXPECT_EQ(err, ""sv);
	}

	std::filesystem::path unique_name(std::string_view name) {
		auto const basename = std::filesystem::temp_directory_path() / name;
		auto filename = basename;
		auto counter = 0u;

		while (exists(filename)) {
			++counter;
			filename = fmt::format("{}-{}", basename, counter);
		}

		return filename;
	}

	std::filesystem::path unique(std::string_view name, std::string_view contents = {}) {
		auto filename = unique_name(name);

		std::ofstream out{filename, std::ios::out | std::ios::binary};
		out.write(contents.data(), static_cast<std::streamsize>(contents.size()));

		return filename;
	}

	TEST(partial, file_not_readable) {
		using namespace std::filesystem;

#ifdef _WIN32
		auto const filename = unique_name("write_only_file"sv);
		create_directories(filename);
#else
		auto const filename = unique("write_only_file"sv);
		permissions(filename, perms::owner_read | perms::group_read | perms::others_read, perm_options::remove);
#endif
		EXPECT_DEATH(partial::config::load_partial(filename), "Quick-DRA: error: could not read .*write_only_file.*\n");
		remove(filename);
	}

	TEST(partial, errors_encountered) {
		auto const filename = unique("version-is-john"sv, "wersja: John\n"sv);
		EXPECT_DEATH(partial::config::load_partial(filename),
		             "Quick-DRA: error: .*version-is-john.* needs to be "
		             "updated before continuing\n");
		remove(filename);
	}

	TEST(partial, fully_read) {
		auto const filename = test_data_dir() / ".quick_dra.AB4123456_50671500000.quarter.yaml"sv;
		::testing::internal::CaptureStderr();
		partial::config::load_partial(filename);
		auto err = ::testing::internal::GetCapturedStderr();

		EXPECT_EQ(err, ""sv);
	}

	TEST(partial, store_current) {
		auto const filename = unique("partial-config-store"sv);
		auto cfg = partial::config::load_partial(test_data_dir() / ".quick_dra.AB4123456_50671500000.quarter.yaml"sv);
		EXPECT_TRUE(cfg.store(filename));
		std::filesystem::remove(filename);
	}

	bool v1_store(v1::partial::config& self,
	              std::filesystem::path const& path,
	              syntax_type syntax = syntax_type::yaml) {
		self.prepare_for_write();
		ryml::Tree tree{};
		auto ref = tree.rootref();
		yaml::write_value(ref, self);

		ryml::csubstr output = yaml::emit_root(syntax, tree, ryml::substr{}, /*error_on_excess*/ false);

		std::vector<char> buf(output.len);
		output = yaml::emit_root(syntax, tree, ryml::to_substr(buf),
		                         /*error_on_excess*/ true);

		std::ofstream out{path, std::ios::out | std::ios::binary};
		if (!out) {
			return false;
		}

		out.write(output.data(), static_cast<std::streamsize>(output.size()));
		return true;
	}

	TEST(partial, store_v1) {
		auto const filename = unique("partial-config-store"sv);
		v1::partial::config cfg{};
		cfg.version = v1::kApiVersion;
		cfg.payer = {
		    {
		        .last_name = "Nowak",
		        .id_card = {},
		        .passport = {},
		        .first_name = "Jan",
		        .kind = "2",
		        .document = "AB4123456",
		    },
		    "7680002466",
		    "26211012346",
		};
		cfg.insured.emplace();
		auto const title = insurance_title{
		    .title_code = "0110",
		    .pension_right = 0,
		    .disability_level = 0,
		};
		auto const piotr_iksinski = v1::partial::insured_t{{
		                                                       .last_name = "Iksiński",
		                                                       .id_card = {},
		                                                       .passport = {},
		                                                       .first_name = "Piotr",
		                                                       .kind = "P",
		                                                       .document = "50671500000",
		                                                   },
		                                                   title,
		                                                   std::nullopt,
		                                                   ratio{1, 4},
		                                                   std::nullopt};
		auto const jan_iksinski = v1::partial::insured_t{{
		                                                     .last_name = "Iksiński",
		                                                     .id_card = {},
		                                                     .passport = {},
		                                                     .first_name = "Jan",
		                                                     .kind = "2",
		                                                     .document = "EH0123456",
		                                                 },
		                                                 title,
		                                                 std::nullopt,
		                                                 ratio{3, 4},
		                                                 9000_PLN};
		auto const name_surname = v1::partial::insured_t{{
		                                                     .last_name = "Surname",
		                                                     .id_card = {},
		                                                     .passport = {},
		                                                     .first_name = "Name",
		                                                     .kind = "1",
		                                                     .document = "AAA000000",
		                                                 },
		                                                 title,
		                                                 std::nullopt,
		                                                 std::nullopt,
		                                                 4500_PLN};
		cfg.insured->push_back(piotr_iksinski);
		cfg.insured->push_back(jan_iksinski);
		cfg.insured->push_back(name_surname);
		EXPECT_TRUE(v1_store(cfg, filename));
		static constexpr auto expected_content = R"(wersja: 1
płatnik:
  nazwisko: 'Nowak, Jan'
  paszport: AB4123456
  nip: 7680002466
  pesel: 26211012346
ubezpieczeni:
  - nazwisko: 'Iksiński, Piotr'
    tytuł ubezpieczenia: 0110 0 0
    pesel: 50671500000
    wymiar: 1/4
  - nazwisko: 'Iksiński, Jan'
    paszport: EH0123456
    tytuł ubezpieczenia: 0110 0 0
    wymiar: 3/4
    pensja: 9000 zł
  - nazwisko: 'Surname, Name'
    dowód: AAA000000
    tytuł ubezpieczenia: 0110 0 0
    pensja: 4500 zł
)"sv;
		auto const actual_content = get_file_contents(filename);
		ASSERT_TRUE(actual_content);
		EXPECT_EQ(expected_content, *actual_content);
		std::filesystem::remove(filename);
	}

	TEST(partial, store_error) {
		auto const dirname = test_data_dir();
		auto cfg = partial::config::load_partial(test_data_dir() / ".quick_dra.AB4123456_50671500000.quarter.yaml"sv);
		EXPECT_FALSE(cfg.store(dirname));
	}
}  // namespace quick_dra::testing
