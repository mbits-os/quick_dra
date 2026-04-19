// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

namespace yaml {
	template <concepts::RequiredType Same>
	inline void take_from(Same& tgt, Same const& src) {
		tgt = src;
	}

	template <typename T, typename S>
	inline void take_from(std::optional<T>& tgt, std::optional<S> const& src) {
		if (src) {
			tgt.emplace();
			take_from(*tgt, *src);
		} else {
			tgt.reset();
		}
	}

	template <typename T, typename S>
	inline void take_from(std::vector<T>& tgt, std::vector<S> const& src) {
		tgt.clear();
		tgt.reserve(src.size());

		std::transform(src.begin(), src.end(), std::back_inserter(tgt), [](auto const& src_item) {
			T result{};
			take_from(result, src_item);
			return result;
		});  // GCOV_EXCL_LINE[GCC]
	}
}  // namespace yaml
