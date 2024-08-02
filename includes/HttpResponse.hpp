#pragma once

#include "Webserv.hpp"
#include "HttpRequest.hpp"
#include "HttpStatusCodes.hpp"
#include "MimeTypes.hpp"
#include "Pipe.hpp"
#include "CgiHandler.hpp"
#include "Location.hpp"
#include "Server.hpp"

/* 
	Takes a string object that contain the whole request message and parse it into 3 Variables 
	_request_line, _request_headers, _request_body. And build the response message.
*/
class Server;
class HttpResponse
{
	public:
		HttpResponse(Server &server);
		HttpResponse(Server &server, HttpRequest &request);
		~HttpResponse();

		void setRequest(HttpRequest &request);
		void setServer(Server &server); // possibly redundant.

		std::string getResponse();
		size_t getBodyLength();
		HttpStatusCodes getErrorCode() const;

		void buildResponse();
		void reset();
		// void cutResponse();
		// int getCgistate();
		// void setCgistate(int state);
		// void setResponse(HttpStatusCodes code);
		std::string getHeader(); //redundant?
		const char *getBody(); // redundant?

		CgiHandler cgiHandler;

		// std::string removeBoundary(std::string &body, std::string &boundary);
		// std::string responseContent;

	private:
		Server &_server;
		HttpRequest _request;

		std::string _requestHeaders; // This is temp only, Later to be changed to something like map<Header_name, Header details>

		std::vector<uint8_t> _responseContent; //possibly change to string ot sure yet.
		size_t _responseBodyLength;

		std::string _responseHeader;
		HttpStatusCodes _statusCode;

		// void tokenize(std::string&, std::string&, std::string del = "\n");

		// new code refactor to dynamic response:
		std::string _targetFile;
		std::string _location;
		bool _cgi;
		Pipe _cgiPipe;
		size_t cgiResponseSize; //redundant?
		bool _autoIndex;

		bool buildBody();
		bool requestIsSuccessful();
		void setStatus();
		void setHeaders();
		//seterrorpage TODO implement server ref in thi sclass to access the errorpages.
		bool readFile();
		void appendContentTypeHeader();
		void appendContentLengthHeader();
		void appendConnectionTypeHeader();
		void appendServerHeader();
		void appendLocationHeader(); // redirectheader.
		void appendDateHeader();

		bool handleTarget(); //is responsible for processing an HTTP request's target resource and determining the appropriate response based on various conditions.
		bool handleCgi(Location &location);
		bool handleCgiTemp(Location &location); // to be renamed

		std::string getLocationMatch(const std::string &path, const std::unordered_map<std::string, Location> &locations); // toberanmed
		std::string combinePaths(const std::string &path1, const std::string &path2, const std::string &path3 = "");
};