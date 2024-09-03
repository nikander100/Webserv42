#pragma once
namespace HTTP{
namespace StatusCode {
enum class Code {
	NONE = 0,
	CONTINUE = 100,
	SWITCHING_PROTOCOLS = 101,
	PROCESSING = 102,
	EARLY_HINTS = 103,
	OK = 200,
	CREATED = 201,
	ACCEPTED = 202,
	NON_AUTHORITATIVE_INFORMATION = 203,
	NO_CONTENT = 204,
	RESET_CONTENT = 205,
	PARTIAL_CONTENT = 206,
	MULTI_STATUS = 207,
	ALREADY_REPORTED = 208,
	IM_USED = 226,
	MULTIPLE_CHOICES = 300,
	MOVED_PERMANENTLY = 301,
	FOUND = 302,
	SEE_OTHER = 303,
	NOT_MODIFIED = 304,
	USE_PROXY = 305,
	TEMPORARY_REDIRECT = 307,
	PERMANENT_REDIRECT = 308,
	BAD_REQUEST = 400,
	UNAUTHORIZED = 401,
	PAYMENT_REQUIRED = 402,
	FORBIDDEN = 403,
	NOT_FOUND = 404,
	METHOD_NOT_ALLOWED = 405,
	NOT_ACCEPTABLE = 406,
	PROXY_AUTHENTICATION_REQUIRED = 407,
	REQUEST_TIMEOUT = 408,
	CONFLICT = 409,
	GONE = 410,
	LENGTH_REQUIRED = 411,
	PRECONDITION_FAILED = 412,
	PAYLOAD_TOO_LARGE = 413,
	URI_TOO_LONG = 414,
	UNSUPPORTED_MEDIA_TYPE = 415,
	RANGE_NOT_SATISFIABLE = 416,
	EXPECTATION_FAILED = 417,
	IM_A_TEAPOT = 418,
	MISDIRECTED_REQUEST = 421,
	UNPROCESSABLE_ENTITY = 422,
	LOCKED = 423,
	FAILED_DEPENDENCY = 424,
	TOO_EARLY = 425,
	UPGRADE_REQUIRED = 426,
	PRECONDITION_REQUIRED = 428,
	TOO_MANY_REQUESTS = 429,
	REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
	UNAVAILABLE_FOR_LEGAL_REASONS = 451,
	INTERNAL_SERVER_ERROR = 500,
	NOT_IMPLEMENTED = 501,
	BAD_GATEWAY = 502,
	SERVICE_UNAVAILABLE = 503,
	GATEWAY_TIMEOUT = 504,
	HTTP_VERSION_NOT_SUPPORTED = 505,
	VARIANT_ALSO_NEGOTIATES = 506,
	INSUFFICIENT_STORAGE = 507,
	LOOP_DETECTED = 508,
	NOT_EXTENDED = 510,
	NETWORK_AUTHENTICATION_REQUIRED = 511
};

inline std::string ToString(Code code)
{
	// Use a static unordered_map for efficient lookup
	static const std::unordered_map<Code, std::string> statusMap = {
		{Code::CONTINUE, "Continue"},
		// {StatusCode::SWITCHING_PROTOCOL, "Switching Protocol"},
		{Code::OK, "OK"},
		{Code::CREATED, "Created"},
		{Code::ACCEPTED, "Accepted"},
		{Code::NON_AUTHORITATIVE_INFORMATION, "Non-Authoritative Information"},
		{Code::NO_CONTENT, "No Content"},
		{Code::RESET_CONTENT, "Reset Content"},
		{Code::PARTIAL_CONTENT, "Partial Content"},
		// {StatusCode::MULTIPLE_CHOICE, "Multiple Choice"},
		{Code::MOVED_PERMANENTLY, "Moved Permanently"},
		// {StatusCode::MOVED_TEMPORARILY, "Moved Temporarily"},
		{Code::SEE_OTHER, "See Other"},
		{Code::NOT_MODIFIED, "Not Modified"},
		{Code::TEMPORARY_REDIRECT, "Temporary Redirect"},
		{Code::PERMANENT_REDIRECT, "Permanent Redirect"},
		{Code::BAD_REQUEST, "Bad Request"},
		{Code::UNAUTHORIZED, "Unauthorized"},
		{Code::FORBIDDEN, "Forbidden"},
		{Code::NOT_FOUND, "Not Found"},
		{Code::METHOD_NOT_ALLOWED, "Method Not Allowed"},
		{Code::NOT_ACCEPTABLE, "Not Acceptable"},
		{Code::PROXY_AUTHENTICATION_REQUIRED, "Proxy Authentication Required"},
		{Code::REQUEST_TIMEOUT, "Request Timeout"},
		{Code::CONFLICT, "Conflict"},
		{Code::GONE, "Gone"},
		{Code::LENGTH_REQUIRED, "Length Required"},
		{Code::PRECONDITION_FAILED, "Precondition Failed"},
		{Code::PAYLOAD_TOO_LARGE, "Payload Too Large"},
		{Code::URI_TOO_LONG, "URI Too Long"},
		{Code::UNSUPPORTED_MEDIA_TYPE, "Unsupported Media Type"},
		// {StatusCode::REQUESTED_RANGE_NOT_SATISFIABLE, "Requested Range Not Satisfiable"},
		{Code::EXPECTATION_FAILED, "Expectation Failed"},
		{Code::IM_A_TEAPOT, "I'm a teapot"},
		{Code::MISDIRECTED_REQUEST, "Misdirected Request"},
		{Code::TOO_EARLY, "Too Early"},
		{Code::UPGRADE_REQUIRED, "Upgrade Required"},
		{Code::PRECONDITION_REQUIRED, "Precondition Required"},
		{Code::TOO_MANY_REQUESTS, "Too Many Requests"},
		{Code::REQUEST_HEADER_FIELDS_TOO_LARGE, "Request Header Fields Too Large"},
		{Code::UNAVAILABLE_FOR_LEGAL_REASONS, "Unavailable for Legal Reasons"},
		{Code::INTERNAL_SERVER_ERROR, "Internal Server Error"},
		{Code::NOT_IMPLEMENTED, "Not Implemented"},
		{Code::BAD_GATEWAY, "Bad Gateway"},
		{Code::SERVICE_UNAVAILABLE, "Service Unavailable"},
		{Code::GATEWAY_TIMEOUT, "Gateway Timeout"},
		{Code::HTTP_VERSION_NOT_SUPPORTED, "HTTP Version Not Supported"},
		{Code::VARIANT_ALSO_NEGOTIATES, "Variant Also Negotiates"},
		{Code::INSUFFICIENT_STORAGE, "Insufficient Storage"},
		{Code::NOT_EXTENDED, "Not Extended"},
		{Code::NETWORK_AUTHENTICATION_REQUIRED, "Network Authentication Required"}
	};

	// Attempt to find the status phrase in the map
	auto it = statusMap.find(code);
	return (it != statusMap.end()) ? it->second : "Undefined";
}

// TODO IMPLEMENT NAMESPACE
// #pragma once
// #include <string>
// #include <unordered_map>

// namespace HttpStatus {

// 	enum class Code {
// 		CONTINUE = 100,
// 		SWITCHING_PROTOCOLS = 101,
// 		// ... rest of the codes ...
// 		NETWORK_AUTHENTICATION_REQUIRED = 511
// 	};

// 	const std::unordered_map<int, std::string> codeToString = {
// 		{100, "CONTINUE"},
// 		{101, "SWITCHING_PROTOCOLS"},
// 		// ... rest of the codes ...
// 		{511, "NETWORK_AUTHENTICATION_REQUIRED"}
// 	};

// 	const std::unordered_map<std::string, Code> stringToCode = {
// 		{"CONTINUE", Code::CONTINUE},
// 		{"SWITCHING_PROTOCOLS", Code::SWITCHING_PROTOCOLS},
// 		// ... rest of the codes ...
// 		{"NETWORK_AUTHENTICATION_REQUIRED", Code::NETWORK_AUTHENTICATION_REQUIRED}
// 	};

// 	std::string toString(Code code) {
// 		int codeAsInt = static_cast<int>(code);
// 		if (codeToString.count(codeAsInt) > 0) {
// 			return codeToString.at(codeAsInt);
// 		} else {
// 			throw std::runtime_error("Invalid status code");
// 		}
// 	}

// 	Code fromString(const std::string& str) {
// 		if (stringToCode.count(str) > 0) {
// 			return stringToCode.at(str);
// 		} else {
// 			throw std::runtime_error("Invalid status code string");
// 		}
// 	}
// }
}
}