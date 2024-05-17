#pragma once

#include "Webserv.hpp"

enum HttpMethod
{
	GET,
	POST,
	DELETE,
	EMPTY
};

/* 
Http Message class is an abstract class that will be used by HttpRequest/HttpResponse clasess
*/
class HttpMessage
{
	public:
		void insertData(char *data, size_t size);
		bool parsingCompleted();
		HttpMethod &getMethod();
		void setMethod(HttpMethod &);
		std::string &getHeader(std::string &);
		void setHeader(std::string &, std::string &);
		void printMessage();
		virtual  ~HttpMessage();

		
	protected:
		std::map<std::string, std::string> _requestHeaders;
		char *_request_body;
		HttpMethod _method;
	private:
		HttpMessage();
};