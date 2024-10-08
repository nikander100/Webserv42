
#pragma once

#include "Webserv.hpp"
#include "Method.hpp"
#include "StatusCodes.hpp"

/**
 * @brief Enumeration representing the different parsing states during HTTP request parsing.
 */
enum State {
	Start, Method_Line_Parsed, Header_Parsed, Reading_Body_Data, Reading_Chunk_Size,
	Reading_Chunk_Data, Reading_Multipart_Data, Part_Complete, Complete
};

/* 
	Request Class will be used to parase and store the request. 
	It gets feeded with the request and will flip a flag when parasing is finished.
*/
class Request
{
	public:
		Request();
		virtual ~Request();

		bool feed(const std::string &data);
		bool parsingComplete() const;
		void print() const;
		void reset();
		HTTP::StatusCode::Code errorCode() const;
		bool keepAlive() const;

		const std::string &getServerName() const;
		const Method &getMethod() const;
		std::string &getPath();
		std::string &getQuery();
		const std::string &getFragment() const;
		const std::string &getHeader(const std::string &key) const;
		std::unordered_map<std::string, std::string> getHeaders() const;
		const std::string &getBody() const;
		const std::string &getBoundary() const;

	private:
		State _state;
		Method _method;
		HTTP::StatusCode::Code _statusCode;

		std::string _serverName;
		std::string _path;
		std::string _query;
		std::string _fragment; // Fragment is client side only but we store it for data collection purposes.
		u_int8_t _verMajor, _verMinor;
		size_t _contentLength;
		size_t _chunkSize;
		std::unordered_map<std::string, std::string> _headers;
		std::string _body;
		std::string _boundary;
		size_t _multipartReadLength;
		bool _isMultipart;
		bool _isChunked;

		bool isValidUri(const std::string &uri);
		std::string decodeUri(const std::string &uri);
		bool isValidToken(const std::string &token);
		bool parseRequestLine(const std::string &line);
		bool parseHeader(const std::string &line);
		bool handleHeaders(std::istream &stream);
		bool handleTextData(std::istringstream &stream);
		bool handleBinaryData(std::istringstream &stream);
		bool parseMultipartData(std::istringstream &stream);
		bool _multipartIsBinary;
		bool parseChunkSize(const std::string &line);
};