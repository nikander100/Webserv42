#pragma once

#include "Webserv.hpp"
#include "Server.hpp"
#include "HttpRequest.hpp"
#include "HttpStatusCodes.hpp"
#include "MimeTypes.hpp"
#include "CgiPipe.hpp"

/* 
	Takes a string object that contain the whole request message and parse it into 3 Variables 
	_request_line, _request_headers, _request_body. And build the response message.
*/
class HttpResponse
{
	public:
		HttpResponse(Server &server);
		HttpResponse(Server &server, HttpRequest &request);
		~HttpResponse();

		void setRequest(HttpRequest &request);
		void setServer(Server &server);

		std::string getResponse();
		size_t getBodyLength();
		HttpStatusCodes getErrorCode() const;

		void buildResponse();
		void reset();
		// void handleCgi();
		// void cutResponse();
		// int getCgistate();
		// void setCgistate(int state);
		// void setResponse(HttpStatusCodes code);
		std::string getHeader(); //redundant?
		const char *getBody(); // redundant?

		// CgiHander cgi_obj;

		// std::string removeBoundary(std::string &body, std::string &boundary);
		// std::string responseContent;

	private:
		Server &_server;
		HttpRequest _request;

		std::string _requestHeaders; // This is temp only, Later to be changed to something like map<Header_name, Header details>
		// std::map<std::string, std::string> _requset_heaeders;

		std::vector<uint8_t> _responseContent; //possibly change to string ot sure yet.
		size_t _responseBodyLength;

		std::string _responseHeader;
		HttpStatusCodes _errorCode;

		// void tokenize(std::string&, std::string&, std::string del = "\n");

		// new code refactor to dynamic response:
		std::string _targetFile;
		std::string _location;
		bool _cgi;
		CgiPipe _cgiPipe;
		size_t cgiResponseSize; //redundant?
		bool _autoindex;

		bool buildBody();
		void setStatus();
		void setHeaders();
		//seterrorpage TODO implement server ref in thi sclass to access the errorpages.
		void readFile();
		void appendContentTypeHeader();
		void appendContentLengthHeader();
		void appendConnectionTypeHeader();
		void appendServerHeader();
		void appendLocationHeader(); // redirectheader.
		void appendDateHeader();

		void handleTarget(); //is responsible for processing an HTTP request's target resource and determining the appropriate response based on various conditions.
		void handleCgi();
		void handleCgiTemp(); // to be renamed
};