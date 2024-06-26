
#pragma once

#include "Webserv.hpp"
#include "Method.hpp"

/**
 * @brief Enumeration representing the different parsing states during HTTP request parsing.
 */
enum State {
	Start, Method_Line_Parsed, Header_Parsed, Reading_Body_Data, Reading_Chunk_Size,
	Reading_Chunk_Data, Complete
};

/* 
	HttpRequest Class will be used to parase and store the request. 
	It gets feeded with the request and will flip a flag when parasing is finished.
*/
class HttpRequest
{
	public:
		HttpRequest();
		virtual ~HttpRequest();

		bool feed(const std::string &data);
		bool parsingComplete() const;
		void print() const;
		void reset();
		int errorCode() const;
		bool keepAlive() const;

		const Method &getMethod() const;
		const std::string &getPath() const;
		const std::string &getQuery() const;
		const std::string &getFragment() const;
		const std::string &getHeader(const std::string &key) const;
		// std::unordered_map<std::string, std::string> getHeaders() const;


	private:
		State _state;
		Method _method;
		int _errorCode;
		bool _flagRequestMethodAndHeaderDone; // can possibly be removed
		bool _flagBody; // can possibly be removed
		bool _flagBodyDone; // can possibly be removed

		std::string _path;
		std::string _query;
		std::string _fragment; // Fragment is client side only but we store it for data collection purposes.
		u_int8_t _verMajor, _verMinor;
		size_t _contentLength;
		size_t _chunkSize;
		std::unordered_map<std::string, std::string> _headers;
		std::string _body;
		// std::vector<std::string> _bodyChunks;
		// std::vector<u_int8_t> body;

		bool isValidUri(const std::string &uri);
		bool isValidToken(const std::string &token);
		bool parseRequestLine(const std::string &line);
		bool parseHeader(const std::string &line);
		// void parseBodyData(const std::string &data, size_t &pos);
		bool parseChunkSize(const std::string &line);

		// void handleHeaders();
};