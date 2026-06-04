// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QByteArray>
#include <quick_dra/base/str.hpp>

namespace quick_dra {
	inline auto as_u8v(QByteArray const& bytes) {
		return as_u8v(std::string_view{bytes.data(), static_cast<size_t>(bytes.size())});
	}

	inline auto strip_sv(QByteArray const& bytes) {
		return strip_sv(std::string_view{bytes.data(), static_cast<size_t>(bytes.size())});
	}
}  // namespace quick_dra
