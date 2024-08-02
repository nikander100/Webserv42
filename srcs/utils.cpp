#include <unordered_map>
#include <string>
#include "../includes/HttpStatusCodes.hpp"

std::string statusCodeToString(HttpStatusCodes statusCode)
{
	// Use a static unordered_map for efficient lookup
	static const std::unordered_map<HttpStatusCodes, std::string> statusMap = {
		{HttpStatusCodes::CONTINUE, "Continue"},
		// {HttpStatusCodes::SWITCHING_PROTOCOL, "Switching Protocol"},
		{HttpStatusCodes::OK, "OK"},
		{HttpStatusCodes::CREATED, "Created"},
		{HttpStatusCodes::ACCEPTED, "Accepted"},
		{HttpStatusCodes::NON_AUTHORITATIVE_INFORMATION, "Non-Authoritative Information"},
		{HttpStatusCodes::NO_CONTENT, "No Content"},
		{HttpStatusCodes::RESET_CONTENT, "Reset Content"},
		{HttpStatusCodes::PARTIAL_CONTENT, "Partial Content"},
		// {HttpStatusCodes::MULTIPLE_CHOICE, "Multiple Choice"},
		{HttpStatusCodes::MOVED_PERMANENTLY, "Moved Permanently"},
		// {HttpStatusCodes::MOVED_TEMPORARILY, "Moved Temporarily"},
		{HttpStatusCodes::SEE_OTHER, "See Other"},
		{HttpStatusCodes::NOT_MODIFIED, "Not Modified"},
		{HttpStatusCodes::TEMPORARY_REDIRECT, "Temporary Redirect"},
		{HttpStatusCodes::PERMANENT_REDIRECT, "Permanent Redirect"},
		{HttpStatusCodes::BAD_REQUEST, "Bad Request"},
		{HttpStatusCodes::UNAUTHORIZED, "Unauthorized"},
		{HttpStatusCodes::FORBIDDEN, "Forbidden"},
		{HttpStatusCodes::NOT_FOUND, "Not Found"},
		{HttpStatusCodes::METHOD_NOT_ALLOWED, "Method Not Allowed"},
		{HttpStatusCodes::NOT_ACCEPTABLE, "Not Acceptable"},
		{HttpStatusCodes::PROXY_AUTHENTICATION_REQUIRED, "Proxy Authentication Required"},
		{HttpStatusCodes::REQUEST_TIMEOUT, "Request Timeout"},
		{HttpStatusCodes::CONFLICT, "Conflict"},
		{HttpStatusCodes::GONE, "Gone"},
		{HttpStatusCodes::LENGTH_REQUIRED, "Length Required"},
		{HttpStatusCodes::PRECONDITION_FAILED, "Precondition Failed"},
		{HttpStatusCodes::PAYLOAD_TOO_LARGE, "Payload Too Large"},
		{HttpStatusCodes::URI_TOO_LONG, "URI Too Long"},
		{HttpStatusCodes::UNSUPPORTED_MEDIA_TYPE, "Unsupported Media Type"},
		// {HttpStatusCodes::REQUESTED_RANGE_NOT_SATISFIABLE, "Requested Range Not Satisfiable"},
		{HttpStatusCodes::EXPECTATION_FAILED, "Expectation Failed"},
		{HttpStatusCodes::IM_A_TEAPOT, "I'm a teapot"},
		{HttpStatusCodes::MISDIRECTED_REQUEST, "Misdirected Request"},
		{HttpStatusCodes::TOO_EARLY, "Too Early"},
		{HttpStatusCodes::UPGRADE_REQUIRED, "Upgrade Required"},
		{HttpStatusCodes::PRECONDITION_REQUIRED, "Precondition Required"},
		{HttpStatusCodes::TOO_MANY_REQUESTS, "Too Many Requests"},
		{HttpStatusCodes::REQUEST_HEADER_FIELDS_TOO_LARGE, "Request Header Fields Too Large"},
		{HttpStatusCodes::UNAVAILABLE_FOR_LEGAL_REASONS, "Unavailable for Legal Reasons"},
		{HttpStatusCodes::INTERNAL_SERVER_ERROR, "Internal Server Error"},
		{HttpStatusCodes::NOT_IMPLEMENTED, "Not Implemented"},
		{HttpStatusCodes::BAD_GATEWAY, "Bad Gateway"},
		{HttpStatusCodes::SERVICE_UNAVAILABLE, "Service Unavailable"},
		{HttpStatusCodes::GATEWAY_TIMEOUT, "Gateway Timeout"},
		{HttpStatusCodes::HTTP_VERSION_NOT_SUPPORTED, "HTTP Version Not Supported"},
		{HttpStatusCodes::VARIANT_ALSO_NEGOTIATES, "Variant Also Negotiates"},
		{HttpStatusCodes::INSUFFICIENT_STORAGE, "Insufficient Storage"},
		{HttpStatusCodes::NOT_EXTENDED, "Not Extended"},
		{HttpStatusCodes::NETWORK_AUTHENTICATION_REQUIRED, "Network Authentication Required"}
	};

	// Attempt to find the status phrase in the map
	auto it = statusMap.find(statusCode);
	return (it != statusMap.end()) ? it->second : "Undefined";
}
