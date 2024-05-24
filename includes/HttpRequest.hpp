
#pragma once

#include "Webserv.hpp"

enum Method {
	GET, POST, DELETE, UNKNOWN
};

/**
 * @brief Enumeration representing the different parsing states during HTTP request parsing.
 */
enum State {
	Start, Request_Line_Parsed, Header_Parsed, Reading_Body_Data, Reading_Chunk_Size,
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
		void print() const;

	protected:
		State _state;
		Method _method;
		std::string _path;
		u_int8_t _verMajor, _verMinor;
		size_t _contentLength;
		size_t _chunkSize;
		std::unordered_map<std::string, std::string> _headers;
		std::string _body;

	private:
		bool parseRequestLine(const std::string &line);
		bool parseHeader(const std::string &line);
		Method parseMethod(const std::string &method);
		// void parseBodyData(const std::string &data, size_t &pos);
		bool parseChunkSize(const std::string &line);
		void parseChunkData(const std::string &data, size_t &pos);
};
