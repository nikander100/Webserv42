#include "HttpResponse.hpp"

HttpResponse::HttpResponse() {
}

HttpResponse::~HttpResponse() {
}

HttpResponse::HttpResponse(HttpRequest &request) : _request(request) {
	request.print();
}

void HttpResponse::addHeaders() {
	static const std::map<std::string, std::string> mimeTypes = {
		{".html", "text/html"},
		{".jpg", "image/jpeg"},
		{".ico", "image/x-icon"},
		{".css", "text/css"},
		{".svg", "image/svg+xml"},
		{".js", "application/javascript"}
		// Add more MIME types as needed
	};
	DEBUG_PRINT("File is = " << _request.getPath());
	std::string path = _request.getPath();
	std::string extension = path.rfind('.') != std::string::npos ? path.substr(path.rfind('.')) : "";
	std::string mimeType = mimeTypes.count(extension) ? mimeTypes.at(extension) : "text/plain";
	_responseHeader.append("Content-Type: " + mimeType + "\r\n");
	_responseHeader.append("Content-Length: " + std::to_string(_responseBodyLength) + "\r\n");
	_responseHeader.append("\r\n"); // append an extra CRLF to separate headers from body
}

void HttpResponse::buildResponse() {
	if (buildBody()) {
		addStatus();
		addHeaders();
	}
	// Temp for testing only
}

std::string HttpResponse::getHeader() {
	return _responseHeader;
}

const char *HttpResponse::getBody() {
	return _responseBody.data();
}

size_t HttpResponse::getBodyLength() {
	return std::max(_responseBodyLength, _responseBody.size());
}

void HttpResponse::addStatus() {
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

void HttpResponse::readFile() {
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
	_responseBody.resize(_responseBodyLength);
	if (!fin.read(_responseBody.data(), _responseBodyLength)) {
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