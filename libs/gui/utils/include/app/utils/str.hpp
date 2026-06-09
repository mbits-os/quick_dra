// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QByteArray>
#include <quick_dra/base/str.hpp>

namespace quick_dra {
	// GCOV_EXCL_START
	// This became needed after downgrading from Qt 6.11.0 to 6.8.3 LTS and as such will not be covered by test. It
	// stems from changes to `QString::toUtf8` between those version, where in 6.11 the integration between the result
	// and `string_view` was much deeper, than in 6.8.

	inline auto as_sv(QByteArray const& bytes) { return std::string_view{bytes}; }
	inline auto as_u8v(QByteArray const& bytes) { return as_u8v(as_sv(bytes)); }
	inline auto strip_sv(QByteArray const& bytes) {
		return strip_sv(std::string_view{bytes.data(), static_cast<size_t>(bytes.size())});
	}
	// GCOV_EXCL_STOP
}  // namespace quick_dra
