#include "Response.hpp"
#include "Server.hpp"

HttpResponse::HttpResponse(Server &server, int clientFd) : _server(server), _clientFd(clientFd), _targetFile(""), _location(""), _cgi(0), _autoIndex(false) {
}

HttpResponse::HttpResponse(Server &server, HttpRequest &request, int clientFd) : _server(server), _request(request), _clientFd(clientFd), _targetFile(""), _location(""), _cgi(0), _autoIndex(false) {
	request.print();
}

HttpResponse::~HttpResponse() {
}

void HttpResponse::setRequest(HttpRequest &request) {
	_request = request;
}

int HttpResponse::getCgistate() {
	return _cgi;
}

void HttpResponse::setCgistate(int state) {
	_cgi = state;
}

void HttpResponse::appendContentTypeHeader() {

	// Get file path and extract extension
	// std::string path = _request.getPath();
	std::string path = _targetFile;

	// Check if the response is from a CGI script
	if (path.empty() && _cgi) {
		// Set Content-Type based on CGI output or default to text/html
		std::string mimeType = "text/html"; // Default MIME type for CGI responses
		_responseHeader.append("Content-Type: " + mimeType + "\r\n");
		return;
	}

	std::string extension = path.rfind('.') != std::string::npos ? path.substr(path.rfind('.')) : "";
	extension = (_autoIndex && path.back() == '/') ? ".html" : extension;

	// Determine MIME type
	std::string mimeType = (_statusCode == HTTP::StatusCode::Code::OK) ? getMimeTypeFromExtension(extension) : "text/html";

	_responseHeader.append("Content-Type: " + mimeType + "\r\n");
}

void HttpResponse::appendContentLengthHeader() {
	_responseHeader.append("Content-Length: " + std::to_string(getResponseBodyLength()) + "\r\n");
}

void HttpResponse::appendConnectionTypeHeader() {
	if (_request.getHeader("connection") == "keep-alive") {
		_responseHeader.append("Connection: keep-alive\r\n");
	}
}

void HttpResponse::appendServerHeader() {
	_responseHeader.append("Server: CRATIX\r\n");
}

void HttpResponse::appendLocationHeader() {
	if (!_location.empty()) {
		_responseHeader.append("Location: " + _location + "\r\n");
	}
}

void HttpResponse::appendDateHeader() {
	// Get the current time
	time_t rawtime;
	struct tm *timeinfo;
	char buffer[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	// Format the time
	strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S %Z", timeinfo);

	_responseHeader.append("Date: " + std::string(buffer) + "\r\n");
}


void HttpResponse::setHeaders() {
	if (_cgi) {
		appendServerHeader();
		appendDateHeader();
		appendConnectionTypeHeader();
		appendContentLengthHeader();
	} else {
		// Add necessary headers to the response header
		appendContentTypeHeader();
		appendContentLengthHeader();
		appendConnectionTypeHeader();
		appendServerHeader();
		appendLocationHeader();
		appendDateHeader();
	}

	// Add a blank line to separate the headers from the body
	_responseHeader.append("\r\n");

	// Add the headers to the response content
	_responseContent.insert(_responseContent.end(), _responseHeader.begin(), _responseHeader.end());
}

std::string HttpResponse::getHeader() {
	return _responseHeader;
}

const char *HttpResponse::getBody() {
	return reinterpret_cast<char*>(_responseContent.data());
}

size_t HttpResponse::getResponseBodyLength() {
	return _responseBody.size();
}

void HttpResponse::setStatus() {
	if (_cgi) {
		return;
	}
	_responseHeader.append("HTTP/1.1 " + std::to_string(static_cast<int>(_statusCode)) + " ");
	_responseHeader.append(HTTP::StatusCode::ToString(_statusCode) + "\r\n");
}

std::string HttpResponse::getLocationMatch(const std::string &path, const std::unordered_map<std::string, Location> &locations) {
	size_t biggest_match = 0;
	std::string bestMatch;

	for (const auto &pair : locations) {
		const std::string &locPath = pair.first;
		if (path.find(locPath) == 0) {
			if (locPath == "/" || path.length() == locPath.length() || path[locPath.length()] == '/') {
				if (locPath.length() > biggest_match) {
					biggest_match = locPath.length();
					bestMatch = locPath;
				}
			}
		}
	}

	return bestMatch; // Return the best match found
}

std::string HttpResponse::combinePaths(const std::string &path1, const std::string &path2, const std::string &path3){
	std::string result = path1;

	// Ensure there's exactly one slash between path1 and path2
	if (!result.empty() && result.back() != '/') {
		result += '/';
	}
	if (!path2.empty() && path2.front() == '/') {
		result += path2.substr(1);
	} else {
		result += path2;
	}

	// Add path3 if it's not empty
	if (!path3.empty()) {
		if (result.back() != '/') {
			result += '/';
		}
		if (path3.front() == '/') {
			result += path3.substr(1);
		} else {
			result += path3;
		}
	}

	return result;
}

bool HttpResponse::checkAndSetStatusCode(CgiHandler& cgiHandler) {
	if (cgiHandler.getStatusCode() == HTTP::StatusCode::Code::INTERNAL_SERVER_ERROR) {
		_statusCode = cgiHandler.getStatusCode();
		_cgi = 0;
		return false;
	}
	return true;
}

bool HttpResponse::handleCgiTemp(Location &location) {
	std::string path = _targetFile;
	cgiHandler.reset();
	cgiHandler.setCgiPath(path);
	_cgi = 1;
	cgiHandler.initEnv(_request, location);
	if (!checkAndSetStatusCode(cgiHandler)) {
		return false;
	}
	cgiHandler.execute();
	if (!checkAndSetStatusCode(cgiHandler)) {
		return false;
	}
	return true;
}

// TODO rename to processCgi
bool HttpResponse::executeCgi(Location &location) {
	std::string path = _request.getPath().erase(0, _request.getPath().find_first_not_of('/'));
	if (path == "cgi-bin") {
		path = combinePaths(path, location.getIndex());
	}
	else if (path == "cgi-bin/") {
		path = combinePaths(path, location.getIndex());
	}
	// Check if the path has an extension
	size_t pos = path.find(".");
	if (pos == std::string::npos) {
		_statusCode = HTTP::StatusCode::Code::NOT_IMPLEMENTED;
		return false;
	}

	// Get the extension from the path
	std::string extension = path.substr(pos);

	// Check if the extension is supported for CGI execution
	if (extension != ".py" && extension != ".sh") {
		_statusCode = HTTP::StatusCode::Code::NOT_IMPLEMENTED;
		return false;
	}

	// Check if the file exists
	if (FileUtils::getTypePath(path) == FileType::NON_EXISTENT) {
		_statusCode = HTTP::StatusCode::Code::NOT_FOUND;
		return false;
	}

	// Check if the file
	if (FileUtils::checkFile(path, W_OK | X_OK) == -1) {
		_statusCode = HTTP::StatusCode::Code::NOT_FOUND;
		return false;
	}

	// allowed methods
	if (location.getAllowedMethods().find(_request.getMethod()) == location.getAllowedMethods().end()) {
		_statusCode = HTTP::StatusCode::Code::METHOD_NOT_ALLOWED;
		return false;
	}

	cgiHandler.reset();
	cgiHandler.setCgiPath(path);
	_cgi = 1;
	cgiHandler.initEnv(_request, location);
	if (!checkAndSetStatusCode(cgiHandler)) {
		return false;
	}
	cgiHandler.execute();
	if (!checkAndSetStatusCode(cgiHandler)) {
		return false;
	}

	return true;
}

bool HttpResponse::buildCgiBody() {
	// Read cgi output
	std::string cgiOutput = cgiHandler.getCgiOutput();

	// Extract headers and body from CGI output
	size_t headerEnd = cgiOutput.find("\r\n\r\n");
	if (headerEnd != std::string::npos) {
		std::string cgiHeaders = cgiOutput.substr(0, headerEnd + 2);
		_responseHeader.append(cgiHeaders);

		// Extract the body from the CGI output
		std::string cgiBody = cgiOutput.substr(headerEnd + 4);
		_responseBody.assign(cgiBody.begin(), cgiBody.end());
	} else {
		// If no headers are found, treat the entire output as the body
		_responseBody.assign(cgiOutput.begin(), cgiOutput.end());
	}

	std::string responsebody = std::string(_responseBody.begin(), _responseBody.end());
	DEBUG_PRINT(GREEN, "Response body: " + responsebody);
	return true;
}

bool HttpResponse::handleTarget() {
	std::string locationKey = getLocationMatch(_request.getPath(), _server.getLocations());

	if (!locationKey.empty()) {
		Location location = _server.getLocation(locationKey);

		// is allowed method
		if (location.getAllowedMethods().find(_request.getMethod()) == location.getAllowedMethods().end()) {
			_statusCode = HTTP::StatusCode::Code::METHOD_NOT_ALLOWED;
			return false;
		}

		if (_request.getMethod() == Method::POST || _request.getMethod() == Method::PUT) {
			// Check if body is larger than allowed
			if (_request.getBody().length() > location.getMaxBodySize()) {
				_statusCode = HTTP::StatusCode::Code::PAYLOAD_TOO_LARGE;
				return false;
			}
		}

		// If the location to redirect to is specified, update the response to indicate
		// a permanent redirection and prepend a slash to the location if it's missing.
		if (!location.getReturn().empty()) {
			_statusCode = HTTP::StatusCode::Code::MOVED_PERMANENTLY; // Indicate permanent redirection
			_location = location.getReturn();

			// Ensure the location starts with a slash for consistency
			if (_location.front() != '/') {
				_location.insert(_location.begin(), '/');
			}

			return false; // Indicate that the response should not proceed as normal
		}

		//handle cgi
		if (location.isCgiPath()) {
			return executeCgi(location);
		}

		//handle alias
		if (!location.getAlias().empty()) {
			_targetFile = combinePaths(location.getAlias(), _request.getPath().substr(location.getPath().length()));
		}
		else {
			_targetFile = combinePaths(location.getRoot(), _request.getPath());
		}

		// Retrieve the CGI path and extension configurations for the target location
		const auto& cgiConfigs = location.getCgiPathExtensions();

		// Check if there are any CGI configurations for this location
		if (!cgiConfigs.empty()) {
			// Iterate through each CGI configuration
			for (const auto& cgiConfig : cgiConfigs) {
				// cgiConfig.first is the CGI extension, and cgiConfig.second is the CGI path
				// Check if the requested file ends with the CGI extension
				if (_targetFile.rfind(cgiConfig.first) != std::string::npos) {
					return handleCgiTemp(location); // Assuming handleCgiTemp takes the CGI path as an argument
				}
			}
		}

		// check if target is a directory
		if (FileUtils::getTypePath(_targetFile) == FileType::DIRECTORY) {
			if (_targetFile.back() != '/') {
				_statusCode = HTTP::StatusCode::Code::MOVED_PERMANENTLY;
				_location = _request.getPath() + "/";
				return false;
			}

			// Append the index file to the target file if it's a directory
			if (!location.getIndex().empty()) {
				_targetFile += location.getIndex();
			}
			else {
				_targetFile += _server.getIndex();
			}

			// Check if the target file exists
			if (!FileUtils::isFileExistAndReadable(_targetFile, "")) {
				if (location.getAutoindex()) {
					_targetFile.erase(_targetFile.find_last_of('/') + 1);
					_autoIndex = true;
					return true;
				}
				else {
					_statusCode = HTTP::StatusCode::Code::FORBIDDEN;
					return false;
				}
			}

			// Check if the target file is readable
			if (FileUtils::getTypePath(_targetFile) == FileType::DIRECTORY) {
				_statusCode = HTTP::StatusCode::Code::MOVED_PERMANENTLY;
				if (!location.getIndex().empty()) {
					_location = combinePaths(_request.getPath(), location.getIndex());
				}
				else {
					_location = combinePaths(_request.getPath(), _server.getIndex());
				}
				if (_location.back() != '/') {
					_location.insert(_location.end(), '/');

				}
				return false;
			}
		}
	}
	else {
		_targetFile = combinePaths(_server.getRoot(), _request.getPath());
		if (FileUtils::getTypePath(_targetFile) == FileType::DIRECTORY) {
			if (_targetFile.back() != '/') {
				_statusCode = HTTP::StatusCode::Code::MOVED_PERMANENTLY;
				_location = _request.getPath() + "/";
				return false;
			}

			// Append the index file to the target file if it's a directory
			_targetFile += _server.getIndex();

			// Check if the target file exists
			if (!FileUtils::isFileExistAndReadable(_targetFile, "")) {
				_statusCode = HTTP::StatusCode::Code::FORBIDDEN;
				return false;
			}

			//check if target is dir
			if (FileUtils::getTypePath(_targetFile) == FileType::DIRECTORY) {
				_statusCode = HTTP::StatusCode::Code::MOVED_PERMANENTLY;
				_location = combinePaths(_request.getPath(), _server.getIndex());
				if (_location.back() != '/') {
					_location.insert(_location.end(), '/');
				}
				return false;
			}
		}
		// is body larger then allowed
		if (_request.getMethod() == Method::POST || _request.getMethod() == Method::PUT) {
			if (_request.getBody().length() > std::stoul(_server.getClientMaxBodySize())) {
				_statusCode = HTTP::StatusCode::Code::PAYLOAD_TOO_LARGE;
				return false;
			}
		}
	}
	return true;
}

std::string HttpResponse::removeBoundary(std::string &body, const std::string &boundary) {
	std::string buffer;
	std::string new_body;
	std::string filename;
	bool is_boundary = false;
	bool is_content = false;

	if (body.find("--" + boundary) != std::string::npos && body.find("--" + boundary + "--") != std::string::npos) {
		for (size_t i = 0; i < body.size(); i++) {
			buffer.clear();
			while (i < body.size() && body[i] != '\n') {
				buffer += body[i];
				i++;
			}
			if (!buffer.compare("--" + boundary + "--\r")) {
				is_content = true;
				is_boundary = false;
			}
			if (!buffer.compare("--" + boundary + "\r")) {
				is_boundary = true;
			}
			if (is_boundary) {
				if (!buffer.compare(0, 31, "Content-Disposition: form-data;")) {
					size_t start = buffer.find("filename=\"");
					if (start != std::string::npos) {
						size_t end = buffer.find("\"", start + 10);
						if (end != std::string::npos)
							filename = buffer.substr(start + 10, end - (start + 10));
					}
				} else if (!buffer.compare(0, 1, "\r") && !filename.empty()) {
					is_boundary = false;
					is_content = true;
				}
			} else if (is_content) {
				if (!buffer.compare("--" + boundary + "\r")) {
					is_boundary = true;
				} else if (!buffer.compare("--" + boundary + "--\r")) {
					new_body.erase(new_body.end() - 1);
					break;
				} else {
					new_body += (buffer + "\n");
				}
			}
		}
	}
	return new_body;
}

bool HttpResponse::buildBody() {
	if (!handleTarget()) {
		return false;
	}

	if (_cgi) {
		return buildCgiBody();
	}

	if (_autoIndex) {
		return true;
	}

	if (_statusCode != HTTP::StatusCode::Code::NONE && _statusCode != HTTP::StatusCode::Code::OK) {
		return true;
	}

	if (_request.getMethod() == Method::GET || _request.getMethod() == Method::HEAD) {
		if (!readFile()) {
			return false;
		}
		_statusCode = HTTP::StatusCode::Code::OK;
		return true;
	}
	else if (_request.getMethod() == Method::POST || _request.getMethod() == Method::PUT) {
		if (FileUtils::isFileExistAndReadable(_targetFile, "") && _request.getMethod() == Method::POST) {
			_statusCode = HTTP::StatusCode::Code::FORBIDDEN;
			return true;
		}

		std::ofstream fout(_targetFile, std::ios::binary);
		if (!fout) {
			DEBUG_PRINT(RED, "Failed to open file for writing: " + _targetFile);
			_statusCode = HTTP::StatusCode::Code::INTERNAL_SERVER_ERROR;
			return false;
		}

		if (!_request.getBoundary().empty()) {
			std::string tempBody = _request.getBody();
			tempBody = removeBoundary(tempBody, _request.getBoundary());
			fout.write(tempBody.c_str(), tempBody.length());
		}
		else{
			fout.write(_request.getBody().c_str(), _request.getBody().length());
		}
		fout.close();
		_statusCode = HTTP::StatusCode::Code::CREATED;
		return true;
	}
	else if (_request.getMethod() == Method::DELETE) {
		if (!FileUtils::isFileExistAndReadable(_targetFile, "")) {
			_statusCode = HTTP::StatusCode::Code::NOT_FOUND;
			return false;
		}
		if (remove(_targetFile.c_str()) != 0) {
			_statusCode = HTTP::StatusCode::Code::INTERNAL_SERVER_ERROR;
			return false;
		}
		_statusCode = HTTP::StatusCode::Code::NO_CONTENT;
		return true;
	}
	_statusCode = HTTP::StatusCode::Code::METHOD_NOT_ALLOWED;
	return false;
}

bool HttpResponse::readFile() {
	std::ifstream fin(_targetFile, std::ios::binary | std::ios::ate);
	if (!fin) {
		_statusCode = HTTP::StatusCode::Code::NOT_FOUND;
		return false;
	}

	size_t responseBodyLength = static_cast<size_t>(fin.tellg());
	fin.seekg(0, std::ios::beg);

	_responseBody.resize(responseBodyLength);
	
	if (!fin.read(reinterpret_cast<char*>(_responseBody.data()), responseBodyLength)) {
		_statusCode = HTTP::StatusCode::Code::INTERNAL_SERVER_ERROR;
		return false;
	}

	/* DEBUG START */
	if (_targetFile.substr(_targetFile.find_last_of(".")) == ".html" && DEBUG == 1) {
		std::cout << YELLOW << "Response body:" << RESET << std::endl;
		for (char c : _responseBody) {
			std::cout << c;
			break ;
		}
		std::cout << RESET << std::endl;
		std::cout << RESET << std::endl;
	}
	else {
		DEBUG_PRINT("Image size: " << _responseBody.size() << " bytes");
	}
	/* DEBUG END */
	return true;
}

HTTP::StatusCode::Code HttpResponse::getErrorCode() const{
	return _statusCode;
}

bool HttpResponse::requestIsSuccessful() {
	_statusCode = _request.errorCode();
	return _statusCode == HTTP::StatusCode::Code::NONE;
}

void HttpResponse::buildErrorBody() {
	std::pair<bool, std::string> errorPageResult = _server.getErrorPage(_statusCode);

	if (errorPageResult.first) { //builtin
		_responseBody.assign(errorPageResult.second.begin(), errorPageResult.second.end());
	}
	else { // custom
		// HTTP::StatusCode::Code tmpCode; TODO
		if (_statusCode >= HTTP::StatusCode::Code::BAD_REQUEST && _statusCode < HTTP::StatusCode::Code::INTERNAL_SERVER_ERROR) {
			_location = errorPageResult.second;
			if (!_location.starts_with("/")) {
				_location.insert(_location.begin(), '/');
			}
			// tmpCode = _statusCode; TODO
			_statusCode = HTTP::StatusCode::Code::FOUND; //possibly needed. but for bette rpractice should be removed..
		}

		_targetFile = combinePaths(_server.getRoot(), _location);
		HTTP::StatusCode::Code oldCode = _statusCode;
		if (!readFile()) {
			_statusCode = oldCode;
			_responseBody.assign(errorPageResult.second.begin(), errorPageResult.second.end());
		}
		// _statusCode = tmpCode; TODO
	}
}

void HttpResponse::setErrorResponse(HTTP::StatusCode::Code code) {
	_responseContent.clear();
	_responseBody.clear();
	_statusCode = code;
	buildErrorBody();
	setStatus();
	setHeaders();
	_responseContent.insert(_responseContent.end(), _responseBody.begin(), _responseBody.end());
}

bool HttpResponse::buildAutoIndexBody() {
	struct dirent *entityStruct;
	DIR *directory;
	std::string dirListPage;
	
	// Open the directory
	directory = opendir(_targetFile.c_str());
	if (directory == NULL) {
		DEBUG_PRINT(RED, "opendir failed for " + _targetFile);
		return false;
	}

	// Start building the HTML page
	dirListPage.append("<html>\n");
	dirListPage.append("<head>\n");
	dirListPage.append("<title>Index of " + _targetFile + "</title>\n");
	dirListPage.append("</head>\n");
	dirListPage.append("<body>\n");
	dirListPage.append("<h1>Index of " + _targetFile + "</h1>\n");
	dirListPage.append("<table style=\"width:80%; font-size: 15px\">\n");
	dirListPage.append("<hr>\n");
	dirListPage.append("<th style=\"text-align:left\">File Name</th>\n");
	dirListPage.append("<th style=\"text-align:left\">Last Modification</th>\n");
	dirListPage.append("<th style=\"text-align:left\">File Size</th>\n");

	struct stat file_stat;
	std::string file_path;

	// Iterate over each entity in the directory
	while((entityStruct = readdir(directory)) != NULL) {
		if (strcmp(entityStruct->d_name, ".") == 0) {
			continue;
		}

		// Get file path and stats
		file_path = _targetFile + entityStruct->d_name;
		if (stat(file_path.c_str(), &file_stat) == -1) {
			DEBUG_PRINT(RED, "stat failed for " + file_path);
			continue;
		}

		// Build the table row for each file or directory
		dirListPage.append("<tr>\n");
		dirListPage.append("<td><a href=\"" + std::string(entityStruct->d_name));

		if (S_ISDIR(file_stat.st_mode)) {
			dirListPage.append("/");
		}

		dirListPage.append("\">" + std::string(entityStruct->d_name));

		if (S_ISDIR(file_stat.st_mode)) {
			dirListPage.append("/");
		}

		dirListPage.append("</a></td>\n");
		dirListPage.append("<td>" + std::string(ctime(&file_stat.st_mtime)) + "</td>\n");
		dirListPage.append("<td>");

		if (!S_ISDIR(file_stat.st_mode)) {
			dirListPage.append(std::to_string(file_stat.st_size));
		}

		dirListPage.append("</td>\n");
		dirListPage.append("</tr>\n");
	}

	// Finish the HTML page
	dirListPage.append("</table>\n");
	dirListPage.append("<hr>\n");
	dirListPage.append("</body>\n");
	dirListPage.append("</html>\n");

	// Close the directory
	closedir(directory);

	// Insert the HTML content into the response body
	_autoIndexBody.assign(dirListPage.begin(), dirListPage.end());

	return true;
}

// first funtion to work on this is the heart of response.
void HttpResponse::buildResponse() {

	// if true handle error
	if (!requestIsSuccessful() || !buildBody()) {
		buildErrorBody();
	}
	// TODO je moeder cgi correct handling
	else if (_autoIndex) {
		if (!buildAutoIndexBody()) {
			_statusCode = HTTP::StatusCode::Code::INTERNAL_SERVER_ERROR;
			buildErrorBody();
		} 
		else {
			_statusCode = HTTP::StatusCode::Code::OK;
		}
		_responseBody.insert(_responseBody.end(), _autoIndexBody.begin(), _autoIndexBody.end());
	}

	setStatus();
	setHeaders();

	if (_request.getMethod() != Method::HEAD) {
		_responseContent.insert(_responseContent.end(), _responseBody.begin(), _responseBody.end());
	}
}

void HttpResponse::reset() {
	_responseContent.clear();
	_responseBody.clear();
	_responseHeader.clear();
	_autoIndexBody.clear();
	_statusCode = HTTP::StatusCode::Code::NONE;
	_targetFile.clear();
	_location.clear();
	_cgi = 0;
	_autoIndex = false;
}

void HttpResponse::cutResponse(size_t size) {
	_responseContent.resize(size);
}

std::string HttpResponse::getResponse() {
	return std::string(_responseContent.begin(), _responseContent.end());
}