#pragma once

#include "Webserv.hpp"
#include "Request.hpp"
#include "StatusCodes.hpp"
#include "MimeTypes.hpp"
#include "Pipe.hpp"
#include "CgiHandler.hpp"
#include "Location.hpp"
#include "Server.hpp"

/* , response(server, _socket->getFd()) 
	Takes a string object that contain the whole request message and parse it into 3 Variables 
	_request_line, _request_headers, _request_body. And build the response message.
*/
class Server;
class HttpResponse
{
	public:
		HttpResponse(Server &server, int clientFd);
		HttpResponse(Server &server, HttpRequest &request, int clientFd);
		~HttpResponse();

		void setRequest(HttpRequest &request);
		
		std::string getResponse();
		size_t getResponseBodyLength();
		HTTP::StatusCode::Code getErrorCode() const;

		void buildResponse();
		void reset();
		void cutResponse(size_t size);
		int getCgistate();
		void setCgistate(int state);
		std::string getHeader(); //redundant?
		const char *getBody(); // redundant?

		CgiHandler cgiHandler;


	private:
		Server &_server;
		int _clientFd;
		HttpRequest _request;

		std::vector<uint8_t> _responseContent;
		std::vector<uint8_t> _responseBody;
		std::vector<uint8_t> _autoIndexBody;

		std::string _responseHeader;
		HTTP::StatusCode::Code _statusCode;

		// new code refactor to dynamic response:
		std::string _targetFile;
		std::string _location;
		int _cgi;
		size_t cgiResponseSize; //redundant?
		bool _autoIndex;

		bool buildBody();
		void buildErrorBody();
		bool buildAutoIndexBody();
		bool buildCgiBody();
		void setErrorResponse(HTTP::StatusCode::Code code);
		bool requestIsSuccessful();
		void setStatus();
		void setHeaders();
		bool readFile();
		void appendContentTypeHeader();
		void appendContentLengthHeader();
		void appendConnectionTypeHeader();
		void appendServerHeader();
		void appendLocationHeader(); // redirectheader.
		void appendDateHeader();

		bool handleTarget(); //is responsible for processing an HTTP request's target resource and determining the appropriate response based on various conditions.
		bool executeCgi(Location &location);
		bool handleCgiTemp(Location &location); // to be renamed
		bool checkAndSetStatusCode(CgiHandler &cgiHandler);


		std::string getLocationMatch(const std::string &path, const std::unordered_map<std::string, Location> &locations); // toberanmed
		std::string combinePaths(const std::string &path1, const std::string &path2, const std::string &path3 = "");
		std::string removeBoundary(std::string &body, const std::string &boundary);
};