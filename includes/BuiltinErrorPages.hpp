#pragma once
#include <unordered_map>
#include "HttpStatusCodes.hpp"
#define CRLF "\r\n"

// TODO RETURN any of these internal error pages if user has not set a custom error page

namespace BuiltinErrorPages {
	static const char wbs42_http_error_301_page[] =
	"<html>" CRLF
	"<head><title>301 Moved Permanently</title></head>" CRLF
	"<body>" CRLF
	"<center><h1>301 Moved Permanently</h1></center>" CRLF;

	static const char wbs42_http_error_302_page[] =
	"<html>" CRLF
	"<head><title>302 Found</title></head>" CRLF
	"<body>" CRLF
	"<center><h1>302 Found</h1></center>" CRLF;

	static const char wbs42_http_error_303_page[] =
	"<html>" CRLF
	"<head><title>303 See Other</title></head>" CRLF
	"<body>" CRLF
	"<center><h1>303 See Other</h1></center>" CRLF;

	static const char wbs42_http_error_307_page[] =
	"<html>" CRLF
	"<head><title>307 Temporary Redirect</title></head>" CRLF
	"<body>" CRLF
	"<center><h1>307 Temporary Redirect</h1></center>" CRLF;

	static const char wbs42_http_error_308_page[] =
	"<html>" CRLF
	"<head><title>308 Permanent Redirect</title></head>" CRLF
	"<body>" CRLF
	"<center><h1>308 Permanent Redirect</h1></center>" CRLF;

	static const char wbs42_http_error_400_page[] =
	"<html>" CRLF
	"<head><title>400 Bad Request</title></head>" CRLF
	"<body>" CRLF
	"<center><h1>400 Bad Request</h1></center>" CRLF;

	static const char wbs42_http_error_401_page[] =
	"<html>" CRLF
	"<head><title>401 Authorization Required</title></head>" CRLF
	"<body>" CRLF
	"<center><h1>401 Authorization Required</h1></center>" CRLF;

	static const char wbs42_http_error_402_page[] =
	"<html>" CRLF
	"<head><title>402 Payment Required</title></head>" CRLF
	"<body>" CRLF
	"<center><h1>402 Payment Required</h1></center>" CRLF;

	static const char wbs42_http_error_403_page[] =
	"<html>" CRLF
	"<head><title>403 Forbidden</title></head>" CRLF
	"<body>" CRLF
	"<center><h1>403 Forbidden</h1></center>" CRLF;

	static const char wbs42_http_error_404_page[] =
	"<html>" CRLF
	"<head><title>404 Not Found</title></head>" CRLF
	"<body>" CRLF
	"<center><h1>404 Not Found</h1></center>" CRLF;

	static const char wbs42_http_error_405_page[] =
	"<html>" CRLF
	"<head><title>405 Not Allowed</title></head>" CRLF
	"<body>" CRLF
	"<center><h1>405 Not Allowed</h1></center>" CRLF;

	static const char wbs42_http_error_406_page[] =
	"<html>" CRLF
	"<head><title>406 Not Acceptable</title></head>" CRLF
	"<body>" CRLF
	"<center><h1>406 Not Acceptable</h1></center>" CRLF;

	static const char wbs42_http_error_408_page[] =
	"<html>" CRLF
	"<head><title>408 Request Time-out</title></head>" CRLF
	"<body>" CRLF
	"<center><h1>408 Request Time-out</h1></center>" CRLF;

	static const char wbs42_http_error_409_page[] =
	"<html>" CRLF
	"<head><title>409 Conflict</title></head>" CRLF
	"<body>" CRLF
	"<center><h1>409 Conflict</h1></center>" CRLF;

	static const char wbs42_http_error_410_page[] =
	"<html>" CRLF
	"<head><title>410 Gone</title></head>" CRLF
	"<body>" CRLF
	"<center><h1>410 Gone</h1></center>" CRLF;

	static const char wbs42_http_error_411_page[] =
	"<html>" CRLF
	"<head><title>411 Length Required</title></head>" CRLF
	"<body>" CRLF
	"<center><h1>411 Length Required</h1></center>" CRLF;

	static const char wbs42_http_error_412_page[] =
	"<html>" CRLF
	"<head><title>412 Precondition Failed</title></head>" CRLF
	"<body>" CRLF
	"<center><h1>412 Precondition Failed</h1></center>" CRLF;

	static const char wbs42_http_error_413_page[] =
	"<html>" CRLF
	"<head><title>413 Request Entity Too Large</title></head>" CRLF
	"<body>" CRLF
	"<center><h1>413 Request Entity Too Large</h1></center>" CRLF;

	static const char wbs42_http_error_414_page[] =
	"<html>" CRLF
	"<head><title>414 Request-URI Too Large</title></head>" CRLF
	"<body>" CRLF
	"<center><h1>414 Request-URI Too Large</h1></center>" CRLF;

	static const char wbs42_http_error_415_page[] =
	"<html>" CRLF
	"<head><title>415 Unsupported Media Type</title></head>" CRLF
	"<body>" CRLF
	"<center><h1>415 Unsupported Media Type</h1></center>" CRLF;

	static const char wbs42_http_error_416_page[] =
	"<html>" CRLF
	"<head><title>416 Requested Range Not Satisfiable</title></head>" CRLF
	"<body>" CRLF
	"<center><h1>416 Requested Range Not Satisfiable</h1></center>" CRLF;

	static const char wbs42_http_error_421_page[] =
	"<html>" CRLF
	"<head><title>421 Misdirected Request</title></head>" CRLF
	"<body>" CRLF
	"<center><h1>421 Misdirected Request</h1></center>" CRLF;

	static const char wbs42_http_error_429_page[] =
	"<html>" CRLF
	"<head><title>429 Too Many Requests</title></head>" CRLF
	"<body>" CRLF
	"<center><h1>429 Too Many Requests</h1></center>" CRLF;

	static const char wbs42_http_error_494_page[] =
	"<html>" CRLF
	"<head><title>400 Request Header Or Cookie Too Large</title></head>"
	CRLF
	"<body>" CRLF
	"<center><h1>400 Bad Request</h1></center>" CRLF
	"<center>Request Header Or Cookie Too Large</center>" CRLF;

	static const char wbs42_http_error_500_page[] =
	"<html>" CRLF
	"<head><title>500 Internal Server Error</title></head>" CRLF
	"<body>" CRLF
	"<center><h1>500 Internal Server Error</h1></center>" CRLF;

	static const char wbs42_http_error_501_page[] =
	"<html>" CRLF
	"<head><title>501 Not Implemented</title></head>" CRLF
	"<body>" CRLF
	"<center><h1>501 Not Implemented</h1></center>" CRLF;

	static const char wbs42_http_error_502_page[] =
	"<html>" CRLF
	"<head><title>502 Bad Gateway</title></head>" CRLF
	"<body>" CRLF
	"<center><h1>502 Bad Gateway</h1></center>" CRLF;

	static const char wbs42_http_error_503_page[] =
	"<html>" CRLF
	"<head><title>503 Service Temporarily Unavailable</title></head>" CRLF
	"<body>" CRLF
	"<center><h1>503 Service Temporarily Unavailable</h1></center>" CRLF;

	static const char wbs42_http_error_504_page[] =
	"<html>" CRLF
	"<head><title>504 Gateway Time-out</title></head>" CRLF
	"<body>" CRLF
	"<center><h1>504 Gateway Time-out</h1></center>" CRLF;

	static const char wbs42_http_error_505_page[] =
	"<html>" CRLF
	"<head><title>505 HTTP Version Not Supported</title></head>" CRLF
	"<body>" CRLF
	"<center><h1>505 HTTP Version Not Supported</h1></center>" CRLF;

	static const char wbs42_http_error_507_page[] =
	"<html>" CRLF
	"<head><title>507 Insufficient Storage</title></head>" CRLF
	"<body>" CRLF
	"<center><h1>507 Insufficient Storage</h1></center>" CRLF;

	const std::unordered_map<HttpStatusCodes, const char *> errorPages = {
		{HttpStatusCodes::MOVED_PERMANENTLY, wbs42_http_error_301_page},
		{HttpStatusCodes::FOUND, wbs42_http_error_302_page},
		{HttpStatusCodes::SEE_OTHER, wbs42_http_error_303_page},
		{HttpStatusCodes::TEMPORARY_REDIRECT, wbs42_http_error_307_page},
		{HttpStatusCodes::PERMANENT_REDIRECT, wbs42_http_error_308_page},
		{HttpStatusCodes::BAD_REQUEST, wbs42_http_error_400_page},
		{HttpStatusCodes::UNAUTHORIZED, wbs42_http_error_401_page},
		{HttpStatusCodes::PAYMENT_REQUIRED, wbs42_http_error_402_page},
		{HttpStatusCodes::FORBIDDEN, wbs42_http_error_403_page},
		{HttpStatusCodes::NOT_FOUND, wbs42_http_error_404_page},
		{HttpStatusCodes::METHOD_NOT_ALLOWED, wbs42_http_error_405_page},
		{HttpStatusCodes::NOT_ACCEPTABLE, wbs42_http_error_406_page},
		{HttpStatusCodes::REQUEST_TIMEOUT, wbs42_http_error_408_page},
		{HttpStatusCodes::CONFLICT, wbs42_http_error_409_page},
		{HttpStatusCodes::GONE, wbs42_http_error_410_page},
		{HttpStatusCodes::LENGTH_REQUIRED, wbs42_http_error_411_page},
		{HttpStatusCodes::PRECONDITION_FAILED, wbs42_http_error_412_page},
		{HttpStatusCodes::PAYLOAD_TOO_LARGE, wbs42_http_error_413_page},
		{HttpStatusCodes::URI_TOO_LONG, wbs42_http_error_414_page},
		{HttpStatusCodes::UNSUPPORTED_MEDIA_TYPE, wbs42_http_error_415_page},
		{HttpStatusCodes::RANGE_NOT_SATISFIABLE, wbs42_http_error_416_page},
		{HttpStatusCodes::MISDIRECTED_REQUEST, wbs42_http_error_421_page},
		{HttpStatusCodes::TOO_MANY_REQUESTS, wbs42_http_error_429_page},
		{HttpStatusCodes::REQUEST_HEADER_FIELDS_TOO_LARGE, wbs42_http_error_494_page},
		{HttpStatusCodes::INTERNAL_SERVER_ERROR, wbs42_http_error_500_page},
		{HttpStatusCodes::NOT_IMPLEMENTED, wbs42_http_error_501_page},
		{HttpStatusCodes::BAD_GATEWAY, wbs42_http_error_502_page},
		{HttpStatusCodes::SERVICE_UNAVAILABLE, wbs42_http_error_503_page},
		{HttpStatusCodes::GATEWAY_TIMEOUT, wbs42_http_error_504_page},
		{HttpStatusCodes::HTTP_VERSION_NOT_SUPPORTED, wbs42_http_error_505_page},
		{HttpStatusCodes::INSUFFICIENT_STORAGE, wbs42_http_error_507_page}
	};

	inline bool isInternalPage(HttpStatusCodes code) {
		return errorPages.count(code) > 0;
	}

	inline const char *getInternalPage(HttpStatusCodes code) {
		return errorPages.at(code);
	}
}