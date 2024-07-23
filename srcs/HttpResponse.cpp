#include "HttpResponse.hpp"

HttpResponse::HttpResponse(Server &server) : _server(server), _targetFile(""), _location(""), _cgi(false), _autoindex(false) {
}

HttpResponse::HttpResponse(Server &server, HttpRequest &request) : _server(server), _request(request), _targetFile(""), _location(""), _cgi(false), _autoindex(false) {
	request.print();
}

HttpResponse::~HttpResponse() {
}

void HttpResponse::setRequest(HttpRequest &request) {
	_request = request;
}

void HttpResponse::setHeaders() { // temp
	DEBUG_PRINT("File is = " << _request.getPath());

	// Get file path and extract extension
	std::string path = _request.getPath();
	std::string extension = path.rfind('.') != std::string::npos ? path.substr(path.rfind('.')) : "";
	
	// Determine MIME type
	std::string mimeType = getMimeTypeFromExtension(extension);

	// Add necessary headers to the response header
	_responseHeader.append("Content-Type: " + mimeType + "\r\n");
	_responseHeader.append("Content-Length: " + std::to_string(_responseBodyLength) + "\r\n");
	_responseHeader.append("\r\n"); // append an extra CRLF to separate headers from body
}


std::string HttpResponse::getHeader() {
	return _responseHeader;
}

const char *HttpResponse::getBody() {
	return reinterpret_cast<char*>(_responseContent.data());
}

size_t HttpResponse::getBodyLength() {
	return std::max(_responseBodyLength, _responseContent.size());
}

void HttpResponse::setStatus() {
	// For testing Assume no errors for now.
	_responseHeader.append("HTTP/1.1 200 OK\n");
}

bool HttpResponse::buildBody() {
	try {
		readFile();
	} catch (const std::runtime_error& e) {
		std::cerr << "webserv: " << e.what() << std::endl;
		return false;
	}
	return true;
}

void HttpResponse::readFile() { // TODO make dynamic
	std::string file_path = _request.getPath().compare("/") == 0 ? "wwwroot/server_dir/home.html" : "wwwroot/server_dir" + _request.getPath();
	DEBUG_PRINT(GREEN, file_path);
	std::ifstream fin(file_path, std::ios::binary | std::ios::ate);
	if (!fin) {
		std::cerr << "webserv: open error: " << strerror(errno) << std::endl;
		throw std::runtime_error("Failed to open file [" + file_path + "]");
	}

	_responseBodyLength = static_cast<size_t>(fin.tellg());
	DEBUG_PRINT(GREEN, "bodylength in readfile: " << _responseBodyLength);
	fin.seekg(0, std::ios::beg);
	_responseContent.resize(_responseBodyLength);
	if (!fin.read(reinterpret_cast<char*>(_responseContent.data()), _responseBodyLength)) {
		std::cerr << "webserv: read error: " << strerror(errno) << std::endl;
		throw std::runtime_error("Failed to read file");
	}

	/* DEBUG START */
	if (file_path.substr(file_path.find_last_of(".")) == ".html") {
		std::cout << YELLOW << "YEE: ";
		for (char c : _responseContent) {
			std::cout << c;
		}
		std::cout << RESET << std::endl;
	}
	std::cout << RESET << std::endl;
	DEBUG_PRINT("Image size: " << _responseContent.size() << " bytes");
	/* DEBUG END */
}

HttpStatusCodes HttpResponse::getErrorCode() const
{
	return _request.errorCode(); // TODO REMOVE THIS, THIS TS TEMP
	return _errorCode;
}

bool HttpResponse::requestIsSuccessful() {
	_errorCode = _request.errorCode();
	return _errorCode == HttpStatusCodes::NONE;
}

// first funtion to work on this is the heart of response.
void HttpResponse::buildResponse() {

	if (requestIsSuccessful() ||buildBody()) {
		// buildErrorBody()
	}
	if (_cgi) {
		return;
	}
	else if (_autoIndex) {
		// build autoindex body
		// if fail set errorcode 500 and build errorbody
		//else  set code 200 and set/insert autoindex to responsebody.
	}
	setStatus();
	setHeaders();
	if (_request.getMethod() != Method::HEAD && (_request.getMethod() == Method::GET || _errorCode != HttpStatusCodes::OK)) {
		//_responseContent.append(_response_body);
	}
}