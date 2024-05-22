#pragma once

#include "Webserv.hpp"

enum HttpMethod
{
	GET,
	POST, 
	DELETE, 
};

enum ParsingState
{
	Request_Line,
	Request_Line_Method,
	Request_Line_First_Space,
	Request_Line_Path_First_Char, //unhandled
	Request_Line_Before_URI,
	Request_Line_URI,
	Request_Line_Ver,
	Request_Line_HT,
	Request_Line_HTT,
	Request_Line_HTTP,
	Request_Line_HTTP_Slash,
	Request_Line_Major,
	Request_Line_Dot,
	Request_Line_Minor,
	Request_Line_CR,
	Request_Line_LF,
	H_Key,
	H_Value,
	Body //unhandled
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
		void feed(char *data, size_t size);
		void feed(const std::string &data);
		bool parsingCompleted();
		HttpMethod &getMethod();
		void  setMethod(HttpMethod &);
		std::string &getHeader(std::string &);
		void setHeader(std::string , std::string );
		void printMessage();


		
	protected:
		std::string _path;
		std::map<std::string, std::string> _requestHeaders;
		std::string _requestBody;
		HttpMethod _method;
		ParsingState _state;
		int _wordIndex;
		int _lineNumber;
		bool _completeFlag;
		bool _skip;
		std::string _storage;
		std::string _keyStorage;
		std::map<u_int8_t, std::string> _methodString;
		int _methodIndex;
		u_int8_t _verMajor;
		u_int8_t _verMinor;
};
