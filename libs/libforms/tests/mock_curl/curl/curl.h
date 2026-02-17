// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

// For definitions copied from true libcurl,
// Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.

#include <cstdlib>

struct global_default_value {};
static constexpr auto CURL_GLOBAL_DEFAULT = global_default_value{};

class CURL;

struct curl_header {
	char* name; /* this might not use the same case */
	char* value;
	size_t amount;       /* number of headers using this name  */
	size_t index;        /* ... of this instance, 0 or higher */
	unsigned int origin; /* see bits below */
	void* anchor;        /* handle privately used by libcurl */
};

enum CURLcode : int {
	CURLE_OK,
	CURLE_UNSUPPORTED_PROTOCOL,   /* 1 */
	CURLE_FAILED_INIT,            /* 2 */
	CURLE_URL_MALFORMAT,          /* 3 */
	CURLE_NOT_BUILT_IN,           /* 4 - [was obsoleted in August 2007 for
	                                 7.17.0, reused in April 2011 for 7.21.5] */
	CURLE_COULDNT_RESOLVE_PROXY,  /* 5 */
	CURLE_COULDNT_RESOLVE_HOST,   /* 6 */
	CURLE_COULDNT_CONNECT,        /* 7 */
	CURLE_WEIRD_SERVER_REPLY,     /* 8 */
	CURLE_REMOTE_ACCESS_DENIED,   /* 9 a service was denied by the server
	                                 due to lack of access - when login fails
	                                 this is not returned. */
	CURLE_FTP_ACCEPT_FAILED,      /* 10 - [was obsoleted in April 2006 for
	                                 7.15.4, reused in Dec 2011 for 7.24.0]*/
	CURLE_FTP_WEIRD_PASS_REPLY,   /* 11 */
	CURLE_FTP_ACCEPT_TIMEOUT,     /* 12 - timeout occurred accepting server
	                                 [was obsoleted in August 2007 for 7.17.0,
	                                 reused in Dec 2011 for 7.24.0]*/
	CURLE_FTP_WEIRD_PASV_REPLY,   /* 13 */
	CURLE_FTP_WEIRD_227_FORMAT,   /* 14 */
	CURLE_FTP_CANT_GET_HOST,      /* 15 */
	CURLE_HTTP2,                  /* 16 - A problem in the http2 framing layer.
	                                 [was obsoleted in August 2007 for 7.17.0,
	                                 reused in July 2014 for 7.38.0] */
	CURLE_FTP_COULDNT_SET_TYPE,   /* 17 */
	CURLE_PARTIAL_FILE,           /* 18 */
	CURLE_FTP_COULDNT_RETR_FILE,  /* 19 */
	CURLE_OBSOLETE20,             /* 20 - NOT USED */
	CURLE_QUOTE_ERROR,            /* 21 - quote command failure */
	CURLE_HTTP_RETURNED_ERROR,    /* 22 */
	CURLE_WRITE_ERROR,            /* 23 */
	CURLE_OBSOLETE24,             /* 24 - NOT USED */
	CURLE_UPLOAD_FAILED,          /* 25 - failed upload "command" */
	CURLE_READ_ERROR,             /* 26 - could not open/read from file */
	CURLE_OUT_OF_MEMORY,          /* 27 */
	CURLE_OPERATION_TIMEDOUT,     /* 28 - the timeout time was reached */
	CURLE_OBSOLETE29,             /* 29 - NOT USED */
	CURLE_FTP_PORT_FAILED,        /* 30 - FTP PORT operation failed */
	CURLE_FTP_COULDNT_USE_REST,   /* 31 - the REST command failed */
	CURLE_OBSOLETE32,             /* 32 - NOT USED */
	CURLE_RANGE_ERROR,            /* 33 - RANGE "command" did not work */
	CURLE_OBSOLETE34,             /* 34 */
	CURLE_SSL_CONNECT_ERROR,      /* 35 - wrong when connecting with SSL */
	CURLE_BAD_DOWNLOAD_RESUME,    /* 36 - could not resume download */
	CURLE_FILE_COULDNT_READ_FILE, /* 37 */
	CURLE_LDAP_CANNOT_BIND,       /* 38 */
	CURLE_LDAP_SEARCH_FAILED,     /* 39 */
	CURLE_OBSOLETE40,             /* 40 - NOT USED */
	CURLE_OBSOLETE41,             /* 41 - NOT USED starting with 7.53.0 */
	CURLE_ABORTED_BY_CALLBACK,    /* 42 */
	CURLE_BAD_FUNCTION_ARGUMENT,  /* 43 */
	CURLE_OBSOLETE44,             /* 44 - NOT USED */
	CURLE_INTERFACE_FAILED,       /* 45 - CURLOPT_INTERFACE failed */
	CURLE_OBSOLETE46,             /* 46 - NOT USED */
	CURLE_TOO_MANY_REDIRECTS,     /* 47 - catch endless re-direct loops */
	CURLE_UNKNOWN_OPTION,         /* 48 - User specified an unknown option */
	CURLE_SETOPT_OPTION_SYNTAX,   /* 49 - Malformed setopt option */
	CURLE_OBSOLETE50,             /* 50 - NOT USED */
	CURLE_OBSOLETE51,             /* 51 - NOT USED */
	CURLE_GOT_NOTHING,            /* 52 - when this is a specific error */
	CURLE_SSL_ENGINE_NOTFOUND,    /* 53 - SSL crypto engine not found */
	CURLE_SSL_ENGINE_SETFAILED,   /* 54 - can not set SSL crypto engine as
	                                 default */
	CURLE_SEND_ERROR,             /* 55 - failed sending network data */
	CURLE_RECV_ERROR,             /* 56 - failure in receiving network data */
	CURLE_OBSOLETE57,             /* 57 - NOT IN USE */
	CURLE_SSL_CERTPROBLEM,        /* 58 - problem with the local certificate */
	CURLE_SSL_CIPHER,             /* 59 - could not use specified cipher */
	CURLE_PEER_FAILED_VERIFICATION, /* 60 - peer's certificate or fingerprint
	                                   was not verified fine */
	CURLE_BAD_CONTENT_ENCODING,     /* 61 - Unrecognized/bad encoding */
	CURLE_OBSOLETE62,               /* 62 - NOT IN USE since 7.82.0 */
	CURLE_FILESIZE_EXCEEDED,        /* 63 - Maximum file size exceeded */
	CURLE_USE_SSL_FAILED,           /* 64 - Requested FTP SSL level failed */
	CURLE_SEND_FAIL_REWIND,         /* 65 - Sending the data requires a rewind
	                                   that failed */
	CURLE_SSL_ENGINE_INITFAILED,    /* 66 - failed to initialise ENGINE */
	CURLE_LOGIN_DENIED,             /* 67 - user, password or similar was not
	                                   accepted and we failed to login */
	CURLE_TFTP_NOTFOUND,            /* 68 - file not found on server */
	CURLE_TFTP_PERM,                /* 69 - permission problem on server */
	CURLE_REMOTE_DISK_FULL,         /* 70 - out of disk space on server */
	CURLE_TFTP_ILLEGAL,             /* 71 - Illegal TFTP operation */
	CURLE_TFTP_UNKNOWNID,           /* 72 - Unknown transfer ID */
	CURLE_REMOTE_FILE_EXISTS,       /* 73 - File already exists */
	CURLE_TFTP_NOSUCHUSER,          /* 74 - No such user */
	CURLE_OBSOLETE75,               /* 75 - NOT IN USE since 7.82.0 */
	CURLE_OBSOLETE76,               /* 76 - NOT IN USE since 7.82.0 */
	CURLE_SSL_CACERT_BADFILE,       /* 77 - could not load CACERT file, missing
	                                   or wrong format */
	CURLE_REMOTE_FILE_NOT_FOUND,    /* 78 - remote file not found */
	CURLE_SSH,                      /* 79 - error from the SSH layer, somewhat
	                                   generic so the error message will be of
	                                   interest when this has happened */

	CURLE_SSL_SHUTDOWN_FAILED,      /* 80 - Failed to shut down the SSL
	                                   connection */
	CURLE_AGAIN,                    /* 81 - socket is not ready for send/recv,
	                                   wait till it is ready and try again (Added
	                                   in 7.18.2) */
	CURLE_SSL_CRL_BADFILE,          /* 82 - could not load CRL file, missing or
	                                   wrong format (Added in 7.19.0) */
	CURLE_SSL_ISSUER_ERROR,         /* 83 - Issuer check failed.  (Added in
	                                   7.19.0) */
	CURLE_FTP_PRET_FAILED,          /* 84 - a PRET command failed */
	CURLE_RTSP_CSEQ_ERROR,          /* 85 - mismatch of RTSP CSeq numbers */
	CURLE_RTSP_SESSION_ERROR,       /* 86 - mismatch of RTSP Session Ids */
	CURLE_FTP_BAD_FILE_LIST,        /* 87 - unable to parse FTP file list */
	CURLE_CHUNK_FAILED,             /* 88 - chunk callback reported error */
	CURLE_NO_CONNECTION_AVAILABLE,  /* 89 - No connection available, the
	                                   session will be queued */
	CURLE_SSL_PINNEDPUBKEYNOTMATCH, /* 90 - specified pinned public key did not
	                                   match */
	CURLE_SSL_INVALIDCERTSTATUS,    /* 91 - invalid certificate status */
	CURLE_HTTP2_STREAM,             /* 92 - stream error in HTTP/2 framing layer
	                                 */
	CURLE_RECURSIVE_API_CALL,       /* 93 - an api function was called from
	                                   inside a callback */
	CURLE_AUTH_ERROR,         /* 94 - an authentication function returned an
	                             error */
	CURLE_HTTP3,              /* 95 - An HTTP/3 layer problem */
	CURLE_QUIC_CONNECT_ERROR, /* 96 - QUIC connection error */
	CURLE_PROXY,              /* 97 - proxy handshake error */
	CURLE_SSL_CLIENTCERT,     /* 98 - client-side certificate required */
	CURLE_UNRECOVERABLE_POLL, /* 99 - poll/select returned fatal error */
	CURLE_TOO_LARGE,          /* 100 - a value/data met its maximum */
	CURLE_ECH_REQUIRED,       /* 101 - ECH tried but failed */
	CURL_LAST                 /* never use! */
};

#define CURLoption auto

using curl_write_callback = std::size_t (*)(char* contents,
                                            std::size_t size,
                                            std::size_t nmemb,
                                            void* userp);
#define OPTION(X)                    \
	X(str, char const*)              \
	X(long, long)                    \
	X(bool, bool)                    \
	X(write_fn, curl_write_callback) \
	X(ptr, void*)

#define INFO(X) X(long, long)

#define X(NAME, TYPE) enum CURL_##NAME##_option : int;
OPTION(X)
#undef X

#define X(NAME, TYPE) enum CURL_##NAME##_info : int;
INFO(X)
#undef X

CURLcode curl_global_init(global_default_value);
void curl_global_cleanup();

CURL* curl_easy_init();
void curl_easy_cleanup(CURL*);

#define X(NAME, TYPE) \
	CURLcode curl_easy_setopt(CURL*, CURL_##NAME##_option, TYPE);
OPTION(X)
#undef X

#define X(NAME, TYPE) \
	CURLcode curl_easy_getinfo(CURL*, CURL_##NAME##_info, TYPE*);
INFO(X)
#undef X

CURLcode curl_easy_perform(CURL*);

#define OPT(NAME, TYPE, VALUE) \
	static constexpr auto NAME = CURL_##TYPE##_option{VALUE};

#define NFO(NAME, TYPE, VALUE) \
	static constexpr auto NAME = CURL_##TYPE##_info{VALUE};

OPT(CURLOPT_USERAGENT, str, 1);
OPT(CURLOPT_URL, str, 2);
OPT(CURLOPT_FOLLOWLOCATION, long, 3);
OPT(CURLOPT_WRITEFUNCTION, write_fn, 4);
OPT(CURLOPT_WRITEDATA, ptr, 5);
OPT(CURLOPT_SSL_VERIFYPEER, bool, 6);

NFO(CURLINFO_RESPONSE_CODE, long, 1001);

enum CURLHcode {
	CURLHE_OK,
	CURLHE_BADINDEX,      /* header exists but not with this index */
	CURLHE_MISSING,       /* no such header exists */
	CURLHE_NOHEADERS,     /* no headers at all exist (yet) */
	CURLHE_NOREQUEST,     /* no request with this number was used */
	CURLHE_OUT_OF_MEMORY, /* out of memory while processing */
	CURLHE_BAD_ARGUMENT,  /* a function argument was not okay */
	CURLHE_NOT_BUILT_IN   /* if API was disabled in the build */
};

struct CURLH_HEADER_t {};

static constexpr auto CURLH_HEADER = CURLH_HEADER_t{};

CURLHcode curl_easy_header(CURL* easy,
                           const char* name,
                           size_t index,
                           CURLH_HEADER_t origin,
                           int request,
                           curl_header** hout);
