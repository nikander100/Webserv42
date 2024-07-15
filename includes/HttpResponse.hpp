#pragma once

#include "Webserv.hpp"
#include "HttpRequest.hpp"
#include "HttpStatusCodes.hpp"
#include "MimeTypes.hpp"

/* 
	Takes a string object that contain the whole request message and parse it into 3 Variables 
	_request_line, _request_headers, _request_body. And build the response message.
*/
class HttpResponse
{
	public:
		HttpResponse();
		HttpResponse(HttpRequest &request);
		~HttpResponse();
		void buildResponse();
		std::string getHeader();
		const char *getBody();
		size_t getBodyLength();
		HttpStatusCodes getErrorCode() const;

	private:
		HttpRequest _request;

		std::string _requestLine[3]; // redundant
		std::string _requestHeaders; // This is temp only, Later to be changed to something like map<Header_name, Header details>
		// std::map<std::string, std::string> _requset_heaeders;
		size_t _responseBodyLength;
		std::vector<char> _responseBody;
		std::string _responseHeader;
		// std::string _responseContent;
		HttpStatusCodes _errorCode;

		bool buildBody();
		void addStatus();
		void addHeaders();
		void readFile();
		// void tokenize(std::string&, std::string&, std::string del = "\n");
};