// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <tangle/decode.hpp>
#include <tangle/uri.hpp>

// GCOV_EXCL_START
// This is tested in tangle repo

namespace tangle {
	namespace {
		inline std::string to_string(std::string_view sv) { return {sv.data(), sv.length()}; }
		int default_port(std::string_view scheme) {
#define KNOWN(proto, port) \
	if (scheme == #proto) return port;

			KNOWN(http, 80);  // ~100% of use cases...
			KNOWN(https, 443);

			KNOWN(ftp, 21);
			KNOWN(ssh, 22);
			KNOWN(telnet, 23);

#undef KNOWN
			return -1;
		}

		std::string tolower(std::string s) {
			for (auto& c : s)
				c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

			return s;
		}

		std::vector<std::string_view> path_split(std::string_view path) {
			size_t length = 1;
			for (auto c : path) {
				if (c == '/') ++length;
			}

			std::vector<std::string_view> out;
			out.reserve(length);

			auto slash = path.find('/');
			decltype(slash) prev = 0;
			while (slash != std::string_view::npos) {
				out.emplace_back(path.substr(prev, slash - prev));
				prev = slash + 1;
				slash = path.find('/', prev);
			}
			out.emplace_back(path.substr(prev));

			return out;
		}

		std::string path_join(const std::vector<std::string>& chunks) {
			if (chunks.empty()) return {};

			auto length = chunks.size() - 1;
			for (auto& ch : chunks)
				length += ch.length();

			std::string out;
			out.reserve(length);

			bool first = true;
			for (auto& ch : chunks) {
				if (first)
					first = false;
				else
					out.push_back('/');

				out.append(ch);
			}

			return out;
		}
	}  // namespace

	auth_parts auth_parts::parse(std::string_view auth) {
		auto pos = auth.find('@');
		auto host = pos == std::string_view::npos ? 0 : pos + 1;

		auto colon = auth.rfind(':');
		if (auth.length() > host && auth[host] == '[') {
			// IPv6/IPVFuture...
			auto end = auth.find(']', host);
			if (end == std::string_view::npos) return {};

			if (auth.length() > (end + 1) && auth[end + 1] != ':') return {};

			colon = end + 1;
			if (colon >= auth.length()) colon = std::string_view::npos;
		} else if (colon < host)
			colon = std::string_view::npos;

		auto host_count = colon == std::string_view::npos ? std::string_view::npos : colon - host;

		auth_parts out{};

		if (host) {
			auto userInfo = auth.substr(0, pos);
			auto pass_colon = userInfo.find(':');
			out.user = urldecode(userInfo.substr(0, pass_colon));
			if (pass_colon != std::string_view::npos)
				out.password = urldecode(userInfo.substr(pass_colon + 1), codec::host);
		}

		out.host = urldecode(auth.substr(host, host_count), codec::host);
		if (colon != std::string_view::npos) out.port = urldecode(auth.substr(colon + 1));

		return out;
	}

	std::string auth_parts::string(auth_flag flag) const {
		auto auser_name = urlencode(user);
		auto auser_pass = urlencode(password, codec::host);
		auto ahost = urlencode(host, codec::host);
		auto aport = urlencode(port);

		if (flag == auth_flag::no_userinfo)
			auser_name.clear();
		else if (flag == auth_flag::ui_safe)
			auser_pass.clear();

		if (aport.empty() && auser_name.empty()) return ahost;

		size_t length = ahost.length();
		if (!auser_name.empty()) {
			length += auser_name.length() + 1;
			if (!auser_pass.empty()) length += auser_pass.length() + 1;
		}
		if (!aport.empty()) length += aport.length() + 1;

		std::string out;
		out.reserve(length);

		if (!auser_name.empty()) {
			out.append(auser_name);
			if (!auser_pass.empty()) {
				out.push_back(':');
				out.append(auser_pass);
			}
			out.push_back('@');
		}

		out.append(ahost);

		if (!aport.empty()) {
			out.push_back(':');
			out.append(aport);
		}
		return out;
	}

	std::string search_params::string(query_flag flag) const {
		std::string out;
		bool first = true;
		auto add_prefix = [&, flag] {
			if (first) {
				first = false;
				if (flag == query_flag::start_with_qmark) out.push_back('?');
			} else
				out.push_back('&');
		};
		for (auto& pair : m_values) {
			if (pair.second.empty()) {
				add_prefix();
				out += urlencode(pair.first, codec::query);
				continue;
			}

			auto name = urlencode(pair.first, codec::query) + "=";
			for (auto& value : pair.second) {
				add_prefix();

				out += name + urlencode(value, codec::query);
			}
		}
		return out;
	}

	search_params::flat_list_type search_params::flat_list() const {
		size_t length = 0;
		for (auto& pair : m_values) {
			if (pair.second.empty())
				++length;
			else
				length += pair.second.size();
		}

		search_params::flat_list_type out;
		out.reserve(length);

		for (auto& pair : m_values) {
			if (pair.second.empty()) out.emplace_back(pair.first, std::nullopt);
			for (auto& value : pair.second)
				out.emplace_back(pair.first, value);
		}

		return out;
	}

#define LOOK_FOR(ch)                  \
	do {                              \
		while (*c != (ch) && c < end) \
			++c;                      \
	} while (0)
#define LOOK_FOR2(ch1, ch2)                           \
	do {                                              \
		while (*c != (ch1) && *c != (ch2) && c < end) \
			++c;                                      \
	} while (0)
#define IS(ch) (c < end && *c == (ch))

	search_params search_params::parse(std::string_view query) {
		search_params out;
		if (!query.empty() && query.front() == '?') query = query.substr(1);
		const char* c = query.data();
		const char* end = c + query.length();
		while (c < end) {
			const char* name_start = c;
			LOOK_FOR2('=', '&');
			std::string name = urldecode({name_start, static_cast<size_t>(c - name_start)}, codec::query);

			if (IS('=')) {
				++c;
				const char* value_start = c;
				LOOK_FOR('&');
				out.add(name, urldecode({value_start, static_cast<size_t>(c - value_start)}, codec::query));
			} else
				out.set(name);

			if (!IS('&')) break;

			++c;
		}
		return out;
	}

#undef WS
#undef LOOK_FOR
#undef LOOK_FOR2
#undef IS

	search_params& search_params::add(const std::string& name, const std::string& value) {
		auto it = m_keys.find(name);
		if (it != m_keys.end()) {
			m_values[it->second].second.push_back(value);
			return *this;
		}

		auto const index = m_values.size();

		m_values.push_back({name, {value}});
		m_keys[name] = index;
		return *this;
	}

	search_params& search_params::set(const std::string& name) {
		auto it = m_keys.find(name);
		if (it != m_keys.end()) {
			m_values[it->second].second.clear();
			return *this;
		}

		auto const index = m_values.size();

		m_values.push_back({name, {}});
		m_keys[name] = index;
		return *this;
	}

	search_params& search_params::remove(const std::string& name) {
		auto it = m_keys.find(name);
		if (it == m_keys.end()) return *this;

		auto const index = it->second;
		m_values.erase(std::next(m_values.begin(), static_cast<std::ptrdiff_t>(index)));

		m_keys.erase(name);
		for (auto& key : m_keys) {
			if (key.second > index) --key.second;
		}
		return *this;
	}

	uri::uri() { ensure_fragment(); }
	uri::uri(const uri&) = default;
	uri& uri::operator=(const uri&) = default;

	uri::uri(std::string_view ident) : m_uri{ident.data(), ident.length()} {
		// no need for invalide_scheme, all indexes are ncalc by default
		ensure_fragment();
	}

	uri::uri(std::string&& ident) : m_uri{std::move(ident)} { ensure_fragment(); }

	uri& uri::operator=(std::string_view ident) {
		m_uri = ident;
		invalidate_scheme();
		return *this;
	}

	uri::uri(uri&& other) noexcept : m_uri{std::move(other.m_uri)} {
		other.invalidate_scheme();
		ensure_fragment();
	}

	uri& uri::operator=(uri&& other) noexcept {
		m_uri = std::move(other.m_uri);
		other.invalidate_scheme();
		invalidate_scheme();
		return *this;
	}

	void uri::ensure_scheme() {
		if (m_scheme != ncalc) return;

		m_scheme = npos;
		auto length = m_uri.length();

		auto b = m_uri.data();
		auto c = b;
		auto e = b + length;

		if (c != e && (c + 1) != e && *c == '/' && c[1] == '/') {
			m_scheme = 0;
			return;
		}

		if (c == e || !isalpha(static_cast<unsigned char>(*c))) return;

		++c;
		while (c != e && (isalnum(static_cast<unsigned char>(*c)) || *c == '+' || *c == '-' || *c == '.'))
			++c;

		if (c == e || *c != ':') return;
		++c;

		m_scheme = static_cast<size_type>(c - b);
	}

	void uri::ensure_path() {
		if (m_path != ncalc) return;

		ensure_scheme();

		if (m_scheme == npos) {
			m_path = 0;
			return;
		}

		m_path = m_scheme;

		if (m_scheme + 1 >= m_uri.length() || m_uri[m_scheme] != '/' || m_uri[m_scheme + 1] != '/') return;

		m_path = m_scheme + 2;

		// This seems to be faster, than find_first_of("/?#", m_path)
		auto const length = m_uri.length();
		auto const* data = m_uri.data() + m_path;
		while (m_path < length) {
			switch (*data) {
				case '/':
				case '?':
				case '#':
					return;
			}
			++m_path;
			++data;
		}
	}

	void uri::ensure_query() {
		if (m_query != ncalc) return;

		ensure_path();

		m_query = m_path;

		auto const length = m_uri.length();
		auto data = m_uri.data() + m_query;

		while (m_query < length) {
			switch (*data) {
				case '?':
				case '#':
					return;
			}
			++m_query;
			++data;
		}
	}

	void uri::ensure_fragment() {
		if (m_part != ncalc) return;

		ensure_query();

		m_part = m_query;
		auto const length = m_uri.length();
		auto data = m_uri.data() + m_part;

		while (m_part < length) {
			if (*data == '#') return;
			++m_part;
			++data;
		}
	}

	bool uri::has_scheme() const { return m_scheme != npos; }

	bool uri::is_scheme_relative() const { return m_scheme == 0; }

	bool uri::has_authority() const {
		// standard behaviour of browsers is
		// that the non-scheme URIs start with path-only
		if (m_scheme == npos) return false;

		if (m_path - m_scheme <= 2) return false;  // no space for //

		auto c = m_uri.data();
		return c[m_scheme] == '/' && c[m_scheme + 1] == '/';
	}

	std::string_view substr(std::string_view sv, size_t off, size_t len = std::string_view::npos) {
		return sv.substr(off, len);
	}

	std::string_view uri::scheme() const& {
		if (!has_scheme() || !m_scheme) return {};

		return substr(m_uri, 0, m_scheme - 1);
	}

	std::string_view uri::authority() const& {
		if (!has_scheme() || is_opaque()) return {};

		auto start = m_scheme + 2;
		return substr(m_uri, start, m_path - start);
	}

	auth_parts uri::parsed_authority() const {
		if (!has_scheme() || is_opaque()) return {};

		return auth_parts::parse(authority());
	}

	std::string_view uri::path() const& { return substr(m_uri, m_path, m_query - m_path); }

	std::string_view uri::search() const& { return substr(m_uri, m_query, m_part - m_query); }

	search_params uri::search_params() const { return search_params::parse(search()); }

	std::string_view uri::resource() const& { return substr(m_uri, m_path, m_part - m_path); }

	std::string_view uri::fragment() const& { return substr(m_uri, m_part); }

	void uri::scheme(std::string_view value) {
		if (m_scheme == npos) return;

		if (m_scheme)
			m_uri.replace(0, m_scheme - 1, value.data(), value.length());
		else if (!value.empty()) {
			m_uri.replace(0, 0, value.data(), value.length());
			m_uri.insert(value.length(), 1, ':');
		} else
			return;
		invalidate_scheme();
	}

	void uri::authority(std::string_view value) {
		if (is_opaque()) return;

		auto start = m_scheme + 2;
		m_uri.replace(start, m_path - start, value.data(), value.length());
		invalidate_path();
	}

	void uri::path(std::string_view value) {
		if (has_authority() && (value.empty() || value[0] != '/')) {
			m_uri.replace(m_path, m_query - m_path, "/");
			++m_path;
			m_query = m_path;
		}
		m_uri.replace(m_path, m_query - m_path, value.data(), value.length());
		invalidate_path();  // query -> path due to having possibly taken the
		                    // '/' branch and having ++m_path
	}

	void uri::search(std::string_view value) {
		m_uri.replace(m_query, m_part - m_query, value.data(), value.length());
		invalidate_fragment();
	}

	void uri::fragment(std::string_view value) {
		m_uri.replace(m_part, m_uri.length() - m_part, value.data(), value.length());
	}

	std::string_view remove_filename(std::string_view path) {
		auto find = path.rfind('/');
		if (find == std::string_view::npos) return path;
		return path.substr(0, find + 1);
	}

	uri uri::make_base(const uri& document) {
		if (document.empty()) return document;

		auto tmp = document;
		if (!tmp.has_scheme()) tmp = uri{"http://" + tmp.string()};

		tmp.fragment(std::string_view());
		tmp.search(std::string_view());
		tmp.path(remove_filename(tmp.path()));
		tmp.ensure_query();
		return tmp;
	}

	uri uri::canonical(const uri& identifier, const uri& base, auth_flag flag, server_quirks quirks) {
		if (identifier.has_authority()) {
			if (identifier.has_scheme() && !identifier.is_scheme_relative()) return normal(identifier, flag, quirks);

			if (!base.has_scheme()) return normal(identifier, flag, quirks);
			// base-scheme://ident-auth/ident-path?ident-query#ident-frag
			auto temp = identifier;
			temp.scheme(base.scheme());
			return normal(std::move(temp), flag, quirks);
		}

		if (identifier.has_scheme()) {
			if (!base.has_authority()) return normal(identifier, flag, quirks);

			if (!base.has_scheme() || tolower(to_string(identifier.scheme())) != tolower(to_string(base.scheme())))
				return normal(identifier, flag, quirks);
		}

		// base-scheme://base-auth/base-path?ident-query#ident-frag
		auto temp = base;
		temp.fragment(identifier.fragment());
		temp.search(identifier.search());

		auto path = identifier.path();
		if (!path.empty() && path[0] == '/') return temp.path(path), normal(std::move(temp), flag, quirks);

		auto bpath = base.path();
		if (!bpath.empty())
			return temp.path(to_string(base.path()) + "/" + to_string(path)), normal(std::move(temp), flag, quirks);
		return temp.path(path), normal(std::move(temp), flag, quirks);
	}

	uri uri::normal(uri tmp, auth_flag flag, server_quirks quirks) {
		if (tmp.m_scheme != npos) {
			auto scheme = tolower(to_string(tmp.scheme()));
			tmp.scheme(scheme);
		}

		if (tmp.has_authority()) {
			auto auth = tmp.parsed_authority();
			if (auth.host.empty())  // decoding failed
				return {};

			// HOST:
			// =======================================================================================
			auth.host = tolower(auth.host);

			// is IPv4, IPv6 or reg-name?
			for (auto c : auth.host) {
				if (!isalnum(static_cast<unsigned char>(c)) && c != '-' && c != '.' && c != '[' && c != ']' && c != ':')
					return {};
			}

			// PORT:
			// =======================================================================================
			// empty or digits
			for (auto c : auth.port) {
				if (!isdigit(static_cast<unsigned char>(c))) return {};
			}

			// if default for the scheme, remove
			if (!auth.port.empty()) {
				auto port = atoi(auth.port.data());
				auto def = default_port(tmp.scheme());
				if (port == def) auth.port.clear();
			}

			tmp.authority(auth.string(flag));
		}
		// PATH:
		// =======================================================================================
		if (tmp.has_authority() || !tmp.has_scheme()) {
			// the "://" uris will have paths with slashes for sure;
			// the "no-scheme, no-auth" URIs start with path with slashes (see
			// has_authority()); cannot say that about other paths (e.g.
			// user@server in mailto:user@server)

			auto path = path_split(tmp.path());
			std::vector<std::string> recoded;
			recoded.reserve(path.size());
			for (auto& part : path)
				recoded.push_back(urlencode(urldecode(part, codec::path),
				                            (quirks & server_quirks::no_plus_in_path) == server_quirks::no_plus_in_path
				                                ? codec::common
				                                : codec::path));

			bool absolute_path = (recoded.size() > 1) && recoded.front().empty();

			// URL ended with slash; should still end with slash
			// Also, URL path ended with either xxxx/. or xxxx/..
			// -> after resolving the result should be a "dir"
			bool empty_at_end =
			    (recoded.size() > 1) && (recoded.back().empty() || recoded.back() == "." || recoded.back() == "..");
			decltype(recoded) canon;
			canon.reserve(recoded.size());

			decltype(recoded) overshots;
			for (auto& p : recoded) {
				if (p.empty() || p == ".") continue;

				if (p == "..") {
					if (canon.empty())
						overshots.push_back(std::move(p));
					else
						canon.pop_back();
					continue;
				}

				canon.push_back(std::move(p));
			}
			if (empty_at_end) canon.emplace_back();
			if (absolute_path)
				tmp.path("/" + path_join(canon));
			else {
				canon.insert(canon.begin(), overshots.begin(), overshots.end());
				tmp.path(path_join(canon));
			}
		}

		// QUERY:
		// =======================================================================================
		if (!tmp.search().empty()) {
			tmp.search(tmp.search_params().string());
		}

		return tmp;
	}
}  // namespace tangle

// GCOV_EXCL_STOP
