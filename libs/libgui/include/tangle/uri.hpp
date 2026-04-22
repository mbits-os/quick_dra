// Copyright (c) 2015 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

/**
URI code.
\file
\author Marcin Zdun <mzdun@midnightbits.com>

This class implements the uri type, a structure allowing
manipulation of resource addresses, with some helper
utilities.
*/

#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

// GCOV_EXCL_START
// This is tested in tangle repo

namespace tangle {

	/**
	 * Flags for uri::auth_parts::string
	 */
	enum class auth_flag {
		ui_safe,    /**< Flag for encoding the user info for UI. */
		with_pass,  /**< Flag for encoding the user info for transfer (highly unsafe). */
		no_userinfo /**< Flag for encoding the authority with no user info at all. */
	};

	enum class server_quirks {
		no_quirks,
		no_plus_in_path,
	};

	inline server_quirks operator&(server_quirks lhs, server_quirks rhs) {
		return server_quirks{std::to_underlying(lhs) & std::to_underlying(rhs)};
	}

	/** Flags for uri::params::string */
	enum class query_flag {
		form_urlencoded = false, /**< Flag for encoding fields from a form to place in request body */
		start_with_qmark = true, /**< Flag for encoding fields to be part of an uri */
	};

	/**
	Helper class for reading and setting
	authority component.
	*/
	struct auth_parts {
		std::string user;     /**< User information - name */
		std::string password; /**< User information - password */
		std::string host;     /**< Host name - either IPv4, IPv6 or registered name */
		std::string port;     /**< String representation of the port on host */

		/**
		Parses authority component into its segments.
		\param auth an authority component to be parsed
		\result a parsed authority with parts, if successful; an authority with empty host on error
		*/
		static auth_parts parse(std::string_view auth);

		/**
		Generates an authority component from its members.

		Members, if present, are percent-encoded and joined together.

		\param flag chooses, what to do with user password, if present; by default, the password is not placed in
		            authority string
		\result an authority component, which may be placed inside an uri object
		*/
		std::string string(auth_flag flag = auth_flag::ui_safe) const;
	};

	/**
	Helper class for parsing, updating and setting
	query component.
	*/
	struct search_params {
	public:
		/**
		Parses a query component to a builder
		\param query a query component to be parsed
		\result a parsed query with decoded name/value pairs
		*/
		static search_params parse(std::string_view query);

		/**
		Adds a new name/value pair.
		\param name a name of the field to add
		\param value a value of the field
		\result a builder reference to chain the calls together
		*/
		search_params& add(const std::string& name, const std::string& value);

		/**
		Sets a value-less param name.
		\param name a name of the field to set
		\result a builder reference to chain the calls together
		*/
		search_params& set(const std::string& name);

		/**
		Removes all fields with given name
		\param name a name of the field to remove
		\result a builder reference to chain the calls together
		*/
		search_params& remove(const std::string& name);

		/**
		Builds a resulting query string for URI or for form request.
		\param flag chooses, if the resulting string should start with question mark, or not; by default, it does
		\result encoded string created from all fields in the builder
		*/
		std::string string(query_flag flag = query_flag::start_with_qmark) const;

		/**
		 Represents a pair on param list. The optional value will be set, if
		 corresponding param had value.
		*/
		using flat_param_type = std::pair<std::string, std::optional<std::string>>;

		/**
		 Represents a flat list of all the parameters on a query string. The
		 order of the params will follow the order of add(), set() and
		 remove() calls, and params with the same name preserve the order of
		 values.
		*/
		using flat_list_type = std::vector<flat_param_type>;

		/**
		Creates a list of all fields for individual access.
		\result a vector of name/value pairs
		*/
		flat_list_type flat_list() const;

		/**
		 Represents a param with all values attached. The vector will not be
		 empty, if corresponding param had value.
		*/
		using multi_param_type = std::pair<std::string, std::vector<std::string>>;

		/**
		 Represents a list of all the parameters with all values on a query
		 string. The order of the params will follow the order of add(),
		 set() and remove() calls, with order within sub-vectors also
		 preserved.
		*/
		using multi_list_type = std::vector<multi_param_type>;

		multi_list_type const& multi_list() const noexcept { return m_values; }

		auto begin() const { return m_values.begin(); }
		auto end() const { return m_values.end(); }
		auto size() const { return m_values.size(); }
		auto empty() const { return m_values.empty(); }
		auto find(std::string const& name) const {
			auto it = m_keys.find(name);
			if (it == m_keys.end()) return end();
			return std::next(begin(), static_cast<std::ptrdiff_t>(it->second));
		}

	private:
#ifndef USING_DOXYGEN
		multi_list_type m_values;
		std::unordered_map<std::string, size_t> m_keys;
#endif
	};

	/**
	 * Unified Resource Identifier class.
	 *
	 * Contains API for:
	 * - reading and setting URI components,
	 * - parsing of authority and query components
	 * - canonization and normalization
	 */
	class uri {
	public:
		uri();                          /**< Constructs empty uri */
		uri(const uri&);                /**< Copy-constructs an uri */
		uri(uri&&) noexcept;            /**< Move-constructs an uri */
		uri& operator=(const uri&);     /**< Copy-assigns an uri */
		uri& operator=(uri&&) noexcept; /**< Move-assigns an uri */

		/**
		Constructs an uri from the identifier.
		\param ident a string representing the uri
		*/
		uri(const char* ident) : uri(std::string_view{ident}) {}

		/**
		Constructs an uri from the identifier.
		\param ident a string representing the uri
		*/
		uri(std::string_view ident);

		/**
		Constructs an uri from the identifier.
		\param ident a string representing the uri
		*/
		uri(std::string&& ident);

		/**
		Copies an uri from the identifier.
		\param ident a string representing the uri
		*/
		uri& operator=(std::string_view ident);

		/**
		Checks, if the URI seems to contain a scheme.
		\returns true, if the uri starts with `[a-z][-+.a-z0-9]*:`
		*/
		bool has_scheme() const;

		/**
		Checks, if the URI seems to contain a non-empty scheme.
		\returns true, if the URI has_scheme(), but is scheme-relative
		*/
		bool is_scheme_relative() const;

		/**
		Checks, if the URI seems to contain an authority.
		\returns true, if the non-scheme part starts with `//`
		*/
		bool has_authority() const;

		/**
		Checks, if the URI seems to contain a path without authority.
		\returns true, if the non-scheme part starts with anything, but `//`
		*/
		bool is_opaque() const { return !has_authority(); }

#ifndef USING_DOXYGEN
		// Using any of those with a high probablity result in dangling
		// pointers; the m_uri will probably be long destructed before the view
		// will be used...
		std::string_view scheme() && = delete;
		std::string_view authority() && = delete;
		std::string_view path() && = delete;
		std::string_view query() && = delete;
		std::string_view resource() && = delete;
		std::string_view fragment() && = delete;
#endif

		/**
		Getter for the scheme property.
		\returns scheme, if present
		*/
		std::string_view scheme() const&;

		/**
		Setter for the scheme property.
		\param value new value of the property
		*/
		void scheme(std::string_view value);

		/**
		Getter for the authority property.
		\returns authority, if present
		*/
		std::string_view authority() const&;

		/**
		Getter for the authority property, pre-parsed to auth_parts.
		\returns parsed authority, if present
		*/
		auth_parts parsed_authority() const;

		/**
		Setter for the authority property.
		\param value new value of the property
		*/
		void authority(std::string_view value);

		/**
		Getter for the path property.
		\returns path, if present
		*/
		std::string_view path() const&;

		/**
		Setter for the path property.
		\param value new value of the property
		*/
		void path(std::string_view value);

		/**
		Getter for the search property.
		\returns search, if present
		*/
		std::string_view search() const&;

		/**
		Getter for the search property, pre-parsed to params.
		\returns parsed search params, if present
		*/
		tangle::search_params search_params() const;

		/**
		Setter for the search property.
		\param value new value of the property
		*/
		void search(std::string_view value);

		/**
		Getter for the resource property.
		\returns path and query, if present
		*/
		std::string_view resource() const&;

		/**
		Getter for the fragment property.
		\returns fragment, if present
		*/
		std::string_view fragment() const&;

		/**
		Setter for the fragment property.
		\param value new value of the property
		*/
		void fragment(std::string_view value);

		/**
		Checks if the uri has no characters.
		\returns true, if the underlying objects is empty.
		*/
		bool empty() const noexcept { return m_uri.empty(); }

		/**
		Getter for the underlying object
		\returns an immutable reference to the underlying string
		*/
		const std::string& string() const { return m_uri; }

		bool operator==(uri const& rhs) const noexcept { return m_uri == rhs.m_uri; }

		bool operator==(std::string_view const& rhs) const noexcept { return m_uri == rhs; }

		/**
		Removes the last component of the path.

		Assuming the path uses slashes, this functions creates a copy of an uri with last part of the path removed. If
		the path already ends with slash, does nothing. Also, the function behaves as if the document was an argument
		supplied to the address bar - it will prepend an HTTP protocol, if no protocol is given.

		\param document an uri to convert to base path
		\returns an uri, which can be used in canonical()
		*/
		static uri make_base(const uri& document);

		/**
		Creates a normal uri, as-if the identifier was relative to base.

		If the identifier represents an already absolute hierarchical uri, this function just forwards the argument to
		normal. Otherwise, it tries to create an uri as if it was seen from a document with base as the address.

		\param identifier an url to expand
		\param base base address to calculate the address against
		\param flag chooses, what to do with user password, if present; by default, the password is not placed in
		            authority string
		\returns a fully-expanded and normalized version of the identifier
		*/
		static uri canonical(const uri& identifier,
		                     const uri& base,
		                     auth_flag flag = auth_flag::ui_safe,
		                     server_quirks quirks = server_quirks::no_quirks);

		/**
		 * Normalizes the input.
		 *
		 * All percent-encoded parts are decoded and re-coded. Empty parts are removed. Both host and scheme are
		 * lower-cased. Path is normalized (segments of `"."` and `".."` are removed). Authority is checked against
		 * possible valid values:
		 *
		 *  - if the host is not IPv4, IPv6 or a string looking like a registered name, the process is aborted
		 *
		 *  - the port, if present, is checked to be digits-only
		 *
		 *  - the port is checked against few well known ports and removed if not necessary
		 *
		 * \param identifier uri to normalize
		 * \param flag chooses, what to do with user password, if present; by default, the password is not placed in
		 *             authority string
		 * \returns normalized uri
		 */
		static uri normal(uri identifier,
		                  auth_flag flag = auth_flag::ui_safe,
		                  server_quirks quirks = server_quirks::no_quirks);

		friend std::ostream& operator<<(std::ostream& out, uri const& address) { return out << address.string(); }

	private:
#ifndef USING_DOXYGEN
		std::string m_uri;
		using size_type = std::string::size_type;

		static constexpr size_type ncalc = static_cast<size_type>(-2);
		static constexpr size_type npos = static_cast<size_type>(-1);

		size_type m_scheme = ncalc;
		size_type m_path = ncalc;
		size_type m_query = ncalc;
		size_type m_part = ncalc;

		void ensure_scheme();
		void ensure_path();
		void ensure_query();
		void ensure_fragment();

		void invalidate_fragment() {
			m_part = ncalc;
			ensure_fragment();
		}

		void invalidate_query() {
			m_query = ncalc;
			invalidate_fragment();
		}

		void invalidate_path() {
			m_path = ncalc;
			invalidate_query();
		}

		void invalidate_scheme() {
			m_scheme = ncalc;
			invalidate_path();
		}
#endif
	};
}  // namespace tangle

// GCOV_EXCL_STOP
