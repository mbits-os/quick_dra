// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <cctype>
#include <string>
#include <tangle/decode.hpp>

// GCOV_EXCL_START
// This is tested in tangle repo

namespace tangle {
	namespace {
		inline bool issafe(unsigned char c) { return std::isalnum(c) || c == '-' || c == '.' || c == '_' || c == '~'; }

		inline bool query_issafe(unsigned char c) { return issafe(c) || c == ' '; }

		inline bool path_issafe(unsigned char c) { return issafe(c) || c == '+'; }

		inline bool host_issafe(unsigned char c) { return issafe(c) || c == ':' || c == '[' || c == ']'; }

		inline char write_ch(char c) { return c; }
		inline char query_write_ch(char c) { return c == ' ' ? '+' : c; }
		inline char read_ch(char c) { return c; }
		inline char query_read_ch(char c) { return c == '+' ? ' ' : c; }

		template <typename SafePred, typename AppendPred>
		inline std::string urlencode_impl(std::string_view in, SafePred&& safe, AppendPred&& write_ch) {
			size_t length = in.size();

			auto b = in.data();
			auto e = in.data() + in.size();

			for (auto it = b; it != e; ++it) {
				if (!safe(static_cast<unsigned char>(*it))) length += 2;
			}

			static constexpr char hexes[] = "0123456789ABCDEF";
			std::string out;
			out.reserve(length);

			for (auto it = b; it != e; ++it) {
				auto c = *it;
				if (safe(static_cast<unsigned char>(c))) {
					out += write_ch(c);
					continue;
				}
				out += '%';
				out += hexes[(c >> 4) & 0xF];
				out += hexes[(c) & 0xF];
			}
			return out;
		}

		inline std::string common_urlencode(std::string_view in) { return urlencode_impl(in, issafe, write_ch); }

		inline std::string host_urlencode(std::string_view in) { return urlencode_impl(in, host_issafe, write_ch); }

		inline std::string path_urlencode(std::string_view in) { return urlencode_impl(in, path_issafe, write_ch); }

		inline std::string query_urlencode(std::string_view in) {
			return urlencode_impl(in, query_issafe, query_write_ch);
		}

		inline int hex(char c) {
			switch (c) {
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					return static_cast<unsigned char>(c - '0');
				case 'a':
				case 'b':
				case 'c':
				case 'd':
				case 'e':
				case 'f':
					return c - 'a' + 10;
				case 'A':
				case 'B':
				case 'C':
				case 'D':
				case 'E':
				case 'F':
					return c - 'A' + 10;
			}
			// Line excluded, as this function is ONLY called inside isxdigit
			// check
			return 0;  // GCOV_EXCL_LINE
		}

		template <typename AppendPred>
		std::string urldecode_impl(std::string_view in, AppendPred&& read_ch) {
			std::string out;
			out.reserve(in.size());

			auto const in_len = in.size();
			for (size_t i = 0; i < in_len; ++i) {
				// go inside only, if there is enough space
				if (in[i] == '%' && (i < in_len - 2) && std::isxdigit(in[i + 1]) && std::isxdigit(in[i + 2])) {
					auto c = (hex(in[i + 1]) << 4) | hex(in[i + 2]);
					out.push_back(static_cast<char>(c));
					i += 2;
					continue;
				}
				out += read_ch(in[i]);
			}
			return out;
		}

		inline std::string common_urldecode(std::string_view in) { return urldecode_impl(in, read_ch); }

		inline std::string query_urldecode(std::string_view in) { return urldecode_impl(in, query_read_ch); }
	}  // namespace

	std::string urlencode(std::string_view in, codec alg) {
		switch (alg) {
			case codec::host:
				return host_urlencode(in);
			case codec::path:
				return path_urlencode(in);
			case codec::query:
				return query_urlencode(in);
			default:
				return common_urlencode(in);
		}
	}

	std::string urldecode(std::string_view in, codec alg) {
		switch (alg) {
			case codec::query:
				return query_urldecode(in);
			default:
				return common_urldecode(in);
		}
	}
}  // namespace tangle

// GCOV_EXCL_STOP
