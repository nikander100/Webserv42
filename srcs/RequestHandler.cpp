#include "RequestHandler.hpp"

RequestHandler::RequestHandler(){
}

RequestHandler::~RequestHandler(){
}

/* 
  TODO: Request message to be parsed here.
  Outcome:
  1- string _request_line[3] filled with 1-Method 2-Path 3-HTTP Ver.
  2- map<string, string> _request_headers filled with header titles and header info. as key/value.
  3- string _request_body filled with request body (if expected).
*/
RequestHandler::RequestHandler(std::string request_content)
{
	std::stringstream ss(request_content);

	for (auto i = 0; i != 3; ++i)
		ss >> _requestLine[i];
}


/* 
   Uses both request_line + request_headers to build the correct response.
   Outcome: Build the response message that will be sent back to the client.
*/

void RequestHandler::addHeaders() {
	static const std::map<std::string, std::string> mimeTypes = {
		{".html", "text/html"},
		{".jpg", "image/jpeg"},
		{".ico", "image/x-icon"},
		{".css", "text/css"},
		// Add more MIME types as needed
	};

	DEBUG_PRINT("File is = " << _requestLine[1]);
	std::string extension = _requestLine[1].rfind('.') != std::string::npos ? _requestLine[1].substr(_requestLine[1].rfind('.')) : "";
	std::string mimeType = mimeTypes.count(extension) ? mimeTypes.at(extension) : "text/plain";

	_responseHeader.append("Content-Type: " + mimeType + "\r\n");
	_responseHeader.append("Content-Length: " + std::to_string(_bodyLength) + "\r\n");
	_responseHeader.append("\r\n"); // append an extra CRLF to separate headers from body
}
void    RequestHandler::buildResponse()
{

	buildBody();
	addStatus();
	addHeaders();
	// Temp for testing only
}

std::string RequestHandler::getHeader() {
	return _responseHeader;
}
const char *RequestHandler::getBody() {
	return _responseBody.data();
}
size_t RequestHandler::getBodyLength() {
	return std::max(_bodyLength, _responseBody.size());
}

/* Check if there is any error and assign the correct status code to response message */
void RequestHandler::addStatus()
{
	// For testing Assume no errors for now.
	_responseHeader.append("HTTP/1.1 200 OK\n");
}


size_t file_size(std::string file_path) 
{
	file_path = "wwwroot/server_dir" + file_path;
	std::ifstream fin(file_path, std::ios::binary | std::ios::ate);
	if (!fin) {
		std::cerr << "webserv: open error: " << strerror(errno) << std::endl;
		std::cerr << file_path << std::endl;
		throw std::runtime_error("Failed to open file [" + file_path + "]");
	}

	size_t size = fin.tellg();
	return size;
}

void RequestHandler::buildBody()
{
	readFile();
}

void RequestHandler::readFile()
{
	DEBUG_PRINT(BLUE, "requestline[1]" << _requestLine[1]);
	std::string file_path = _requestLine[1].compare("/") == 0 ? "wwwroot/server_dir/home.html" : "wwwroot/server_dir" + _requestLine[1];
	DEBUG_PRINT(GREEN, file_path);
	std::ifstream fin(file_path, std::ios::binary | std::ios::ate);
	if (!fin) {
		std::cerr << "webserv: open error: " << strerror(errno) << std::endl;
		throw std::runtime_error("Failed to open file [" + file_path + "]");
	}

	_bodyLength = static_cast<size_t>(fin.tellg());
	DEBUG_PRINT(GREEN, "bodylength in readfile: " << _bodyLength);
	fin.seekg(0, std::ios::beg);

	_responseBody.resize(_bodyLength);
	if (!fin.read(_responseBody.data(), _bodyLength)) {
		std::cerr << "webserv: read error: " << strerror(errno) << std::endl;
		throw std::runtime_error("Failed to read file");
	}
	/* DEBUG START */
	if (file_path.substr(file_path.find_last_of(".")) == ".html") {
		std::cout << YELLOW << "YEE: ";
		for (char c : _responseBody) {
			std::cout << c;
		}
		std::cout << RESET << std::endl;
	}
	std::cout << RESET << std::endl;
	DEBUG_PRINT("Image size: " << _responseBody.size() << " bytes");
	/* DEBUG END */
}