// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <quick_dra/base/paths.hpp>

namespace quick_dra::platform {
	namespace {
		std::filesystem::path& _config_data_dir() {
			static auto path = exec_dir().parent_path() / dir_names::config;
			return path;
		}

		std::filesystem::path _home_path() {
			auto const* const HOME = std::getenv("HOME");
			if (HOME) {
				return HOME;
			}
			auto const* const USERPROFILE = std::getenv("USERPROFILE");
			if (USERPROFILE) {
				return USERPROFILE;
			}

			std::filesystem::path result{};

			auto const* const HOMEDRIVE = std::getenv("HOMEDRIVE");
			auto const* HOMEPATH = std::getenv("HOMEPATH");
			if (HOMEDRIVE) {
				result = HOMEDRIVE;
			}
			if (!HOMEPATH) HOMEPATH = "";
			result /= HOMEPATH;

			return result;
		}
	}  // namespace

	std::filesystem::path const& config_data_dir() { return _config_data_dir(); }
	void config_data_dir(std::filesystem::path const& new_value) { _config_data_dir() = new_value; }

	std::filesystem::path const& home_path() {
		static auto const path = _home_path();
		return path;
	}
}  // namespace quick_dra::platform
