// Copyright (c) 2015 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <string>
#include <string_view>

// GCOV_EXCL_START
// This is tested in tangle repo

namespace tangle {
	enum class codec { common, host, path, query };

	/**
	Prepares the input range to be safely used as a part
	of URL.

	Converts all characters to their hex counterparts (%%XX),
	except for the unreserved ones: letters, digits,
	`"-"`, `"."`, `"_"`, `"~"`. This function, together with urldecode,
	can be used for URI normalization.

	\param in a std::string_view to encode
	\param alg encoding algorithm to use
	\return percent-encoded string containing no reserved characters, except for the percent sign.

	\see RFC3986, section 2.1, Percent-Encoding
	\see RFC3986, section 2.3, Unreserved Characters
	\see urldecode(const char*,size_t)
	*/
	std::string urlencode(std::string_view in, codec alg = codec::common);

	/**
	Removes all percent-encodings from the input range.

	Converts all encoded octets in form of %%XX to their
	raw form.

	\param in a std::string_view to decode
	\param alg decoding algorithm to use
	\return percent-decoded string, which may include some
	        registered characters.

	\see RFC3986, section 2.1, Percent-Encoding
	\see tangle::urlencode(const char*,size_t)
	*/
	std::string urldecode(std::string_view in, codec alg = codec::common);
}  // namespace tangle

// GCOV_EXCL_STOP
