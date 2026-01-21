// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <fmt/std.h>
#include <fstream>
#include <optional>
#include <string>
#include <yaml/reader.hpp>

using namespace std::literals;

namespace yaml {
	namespace {
		static bool iequals(std::string_view lhs, std::string_view rhs) {
			if (lhs.size() != rhs.size()) return false;

			for (size_t i = 0; i < lhs.size(); ++i) {
				auto const l = static_cast<unsigned char>(lhs[i]);
				auto const r = static_cast<unsigned char>(rhs[i]);
				if (std::tolower(l) != static_cast<int>(r)) return false;
			}
			return true;
		}
	}  // namespace

	bool read_value(ref_ctx const& ref, bool& ctx) {
		if (!ref.ref().has_val()) {
			ctx = false;
			return false;
		}

		auto const value = ref.val();
		if (value.empty()) {
			ctx = false;
			return true;
		}

		if (iequals(view(value), "true"sv)) {
			ctx = true;
		} else if (iequals(view(value), "false"sv)) {
			ctx = false;
		} else {
			return ref.error("expecting a true or false");
		}

		return true;
	}

	bool read_value(ref_ctx const& ref, std::string& ctx) {
		if (!ref.ref().has_val()) {
			ctx.clear();
			return false;
		}

		auto const value = ref.val();
		if (value.empty()) {
			ctx = {};
			return false;
		}

		ctx = {value.data(), value.size()};
		return true;
	}

	bool convert_string(ref_ctx const&,
	                    c4::csubstr const& value,
	                    std::string& ctx) {
		ctx = view(value);
		return true;
	}
}  // namespace yaml
