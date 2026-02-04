// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <cctype>
#include <iostream>
#include <quick_dra/base/str.hpp>

namespace quick_dra {
	namespace {
		template <typename String>
		struct helper;

		template <>
		struct helper<std::string_view> {
			static inline std::string_view as_str(
			    std::string_view str) noexcept {
				return str;
			}
		};

		template <>
		struct helper<std::string> {
			static inline std::string as_str(std::string_view str) {
				return {str.data(), str.size()};
			}
		};
	}  // namespace

	template <typename String>
	std::vector<String> split_impl(std::string_view data,
	                               sep_view_t sep,
	                               size_t max) {
		std::vector<String> result{};

		if (sep.empty()) {
			data = strip_sv(data);

			if (data.empty()) {
				result.push_back(helper<String>::as_str(""sv));
				return result;
			}

			auto const whitespaces = " \f\t\v\r\n"sv;

			auto pos = data.find_first_of(whitespaces);
			while (max && pos != std::string_view::npos) {
				auto const view = data.substr(0, pos);
				data = lstrip_sv(data.substr(pos));
				pos = data.find_first_of(whitespaces);
				if (max != std::string::npos) --max;

				result.push_back(helper<String>::as_str(view));
			}
			result.push_back(helper<String>::as_str(lstrip_sv(data)));

			return result;
		}

		auto pos = data.find(sep.value);
		decltype(pos) prev = 0;

		while (max && pos != std::string_view::npos) {
			auto const view = data.substr(prev, pos - prev);
			prev = pos + sep.size();
			pos = data.find(sep.value, prev);
			if (max != std::string::npos) --max;

			result.push_back(helper<String>::as_str(view));
		}

		result.push_back(helper<String>::as_str(data.substr(prev)));

		return result;
	}

	size_t calc_separators(sep_char_t sep, std::string_view data, size_t max) {
		auto pos = data.find(sep.value);
		decltype(pos) prev = 0;

		size_t result = 1;

		while (max && pos != std::string_view::npos) {
			prev = pos + 1;
			pos = data.find(sep.value, prev);
			if (max != std::string::npos) --max;

			++result;
		}

		return result;
	}

	template <typename String>
	std::vector<String> split_impl(std::string_view data,
	                               sep_char_t sep,
	                               size_t max) {
		std::vector<String> result{};

		result.reserve(calc_separators(sep, data, max));

		auto pos = data.find(sep.value);
		decltype(pos) prev = 0;

		while (max && pos != std::string_view::npos) {
			auto const view = data.substr(prev, pos - prev);
			prev = pos + 1;
			pos = data.find(sep.value, prev);
			if (max != std::string::npos) --max;

			result.push_back(helper<String>::as_str(view));
		}

		result.push_back(helper<String>::as_str(data.substr(prev)));

		return result;
	}

	std::vector<std::string_view> split_sv(std::string& data,
	                                       sep_view_t sep,
	                                       size_t max) {
		return split_impl<std::string_view>(data, sep, max);
	}

	std::vector<std::string_view> split_sv(std::string_view data,
	                                       sep_view_t sep,
	                                       size_t max) {
		return split_impl<std::string_view>(data, sep, max);
	}

	std::vector<std::string> split_s(std::string_view data,
	                                 sep_view_t sep,
	                                 size_t max) {
		return split_impl<std::string>(data, sep, max);
	}

	std::vector<std::string_view> split_sv(std::string& data,
	                                       sep_char_t sep,
	                                       size_t max) {
		return split_impl<std::string_view>(data, sep, max);
	}

	std::vector<std::string_view> split_sv(std::string_view data,
	                                       sep_char_t sep,
	                                       size_t max) {
		return split_impl<std::string_view>(data, sep, max);
	}

	std::vector<std::string> split_s(std::string_view data,
	                                 sep_char_t sep,
	                                 size_t max) {
		return split_impl<std::string>(data, sep, max);
	}

	std::string_view lstrip_sv(std::string_view data) {
		auto new_stop = data.size();
		decltype(new_stop) new_start = 0;

		while (new_start < new_stop &&
		       std::isspace(static_cast<unsigned char>(data[new_start])))
			++new_start;

		return data.substr(new_start);
	}

	std::string_view rstrip_sv(std::string_view data) {
		auto new_stop = data.size();

		while (new_stop > 0 &&
		       std::isspace(static_cast<unsigned char>(data[new_stop - 1])))
			--new_stop;

		return data.substr(0, new_stop);
	}

	std::string_view strip_sv(std::string_view data) {
		return lstrip_sv(rstrip_sv(data));
	}

	std::string strip_s(std::string_view data) {
		auto const result = strip_sv(data);
		return {result.data(), result.size()};
	}

	std::string lstrip_s(std::string_view data) {
		auto const result = lstrip_sv(data);
		return {result.data(), result.size()};
	}

	std::string rstrip_s(std::string_view data) {
		auto const result = rstrip_sv(data);
		return {result.data(), result.size()};
	}

	template <typename String, typename Sep>
	std::string join_impl(std::vector<String> const& items, Sep sep) {
		std::string result{};

		if (items.empty()) return result;

		auto length = sep.size() * (items.size() - 1);
		for (auto const& item : items)
			length += item.size();

		result.reserve(length);

		auto first = true;

		for (auto const& item : items) {
			if (first)
				first = false;
			else
				sep.append_to(result);
			result.append(item);
		}

		return result;
	}

	std::string join(std::vector<std::string_view> const& items,
	                 sep_view_t sep) {
		return join_impl(items, sep);
	}

	std::string join(std::vector<std::string> const& items, sep_view_t sep) {
		return join_impl(items, sep);
	}

	std::string join(std::vector<std::string_view> const& items,
	                 sep_char_t sep) {
		return join_impl(items, sep);
	}

	std::string join(std::vector<std::string> const& items, sep_char_t sep) {
		return join_impl(items, sep);
	}
}  // namespace quick_dra
