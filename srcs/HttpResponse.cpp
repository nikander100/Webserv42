#include "HttpResponse.hpp"
// #include "Server.hpp"

HttpResponse::HttpResponse(Server &server) : _server(server), _targetFile(""), _location(""), _cgi(false), _autoIndex(false) {
}

HttpResponse::HttpResponse(Server &server, HttpRequest &request) : _server(server), _request(request), _targetFile(""), _location(""), _cgi(false), _autoIndex(false) {
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

bool HttpResponse::handleCgiTemp(Location &location) {
	std::string path = _targetFile;
	cgiHandler.reset();
	cgiHandler.setCgiPath(path);
	_cgi = true;
	cgiHandler.initEnv(_request, location);
	cgiHandler.execute(_statusCode);
	if (_statusCode == HttpStatusCodes::INTERNAL_SERVER_ERROR) { // Would be better practice to check for OK or NONE but as only error that could be returned fron the handler is 500 its simpler to check against that.
		return false;
	}
	return true;
}

bool HttpResponse::handleCgi(Location &location) {
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
		_statusCode = HttpStatusCodes::NOT_IMPLEMENTED;
		return false;
	}

	// Get the extension from the path
	std::string extension = path.substr(pos);

	// Check if the extension is supported for CGI execution
	if (extension != ".py" && extension != ".sh") {
		_statusCode = HttpStatusCodes::NOT_IMPLEMENTED;
		return false;
	}

	// Check if the file exists
	if (FileUtils::getTypePath(path) == FileType::NON_EXISTENT) {
		_statusCode = HttpStatusCodes::NOT_FOUND;
		return false;
	}

	// Check if the file
	if (FileUtils::checkFile(path, W_OK | X_OK) == -1) {
		_statusCode = HttpStatusCodes::NOT_FOUND;
		return false;
	}

	// allowed methods
	if (location.getAllowedMethods().find(_request.getMethod()) == location.getAllowedMethods().end()) {
		_statusCode = HttpStatusCodes::METHOD_NOT_ALLOWED;
		return false;
	}

	cgiHandler.reset();
	cgiHandler.setCgiPath(path);
	_cgi = true;
	cgiHandler.initEnv(_request, location);
	cgiHandler.execute(_statusCode);
	if (_statusCode == HttpStatusCodes::INTERNAL_SERVER_ERROR) { // Would be better practice to check for OK or NONE but as only error that could be returned fron the handler is 500 its simpler to check against that.
		return false;
	}

	return true;
}

bool HttpResponse::handleTarget() {
	std::string locationKey = getLocationMatch(_request.getPath(), _server.getLocations());

	if (!locationKey.empty()) {
		Location location = _server.getLocation(locationKey);

		// is allowed method
		if (location.getAllowedMethods().find(_request.getMethod()) == location.getAllowedMethods().end()) {
			_statusCode = HttpStatusCodes::METHOD_NOT_ALLOWED;
			return false;
		}

		// is body larger then allowed
		if (_request.getBody().length() > location.getMaxBodySize()) {
			_statusCode = HttpStatusCodes::PAYLOAD_TOO_LARGE;
			return false;
		}

		// If the location to redirect to is specified, update the response to indicate
		// a permanent redirection and prepend a slash to the location if it's missing.
		if (!location.getReturn().empty()) {
			_statusCode = HttpStatusCodes::MOVED_PERMANENTLY; // Indicate permanent redirection
			_location = location.getReturn();

			// Ensure the location starts with a slash for consistency
			if (_location.front() != '/') {
				_location.insert(_location.begin(), '/');
			}

			return false; // Indicate that the response should not proceed as normal
		}

		//handle cgi
		if (location.isCgiPath()) {
			return handleCgi(location);
		}

		//handle alias
		if (!location.getAlias().empty()) {
			_targetFile = combinePaths(location.getAlias(), _request.getPath().substr(location.getPath().length()));
		}
		else {
			_targetFile = combinePaths(location.getRoot(), _request.getPath());
		}

		// Retrieve the CGI path and extension configurations for the target location
		const auto& cgiConfigs = location.getCgiPathExtension();

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
				_statusCode = HttpStatusCodes::MOVED_PERMANENTLY;
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
					_statusCode = HttpStatusCodes::FORBIDDEN;
					return false;
				}
			}

			// Check if the target file is readable
			if (FileUtils::getTypePath(_targetFile) == FileType::DIRECTORY) {
				_statusCode = HttpStatusCodes::MOVED_PERMANENTLY;
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
				_statusCode = HttpStatusCodes::MOVED_PERMANENTLY;
				_location = _request.getPath() + "/";
				return false;
			}

			// Append the index file to the target file if it's a directory
			_targetFile += _server.getIndex();

			// Check if the target file exists
			if (!FileUtils::isFileExistAndReadable(_targetFile, "")) {
				_statusCode = HttpStatusCodes::FORBIDDEN;
				return false;
			}

			//check if target is dir
			if (FileUtils::getTypePath(_targetFile) == FileType::DIRECTORY) {
				_statusCode = HttpStatusCodes::MOVED_PERMANENTLY;
				_location = combinePaths(_request.getPath(), _server.getIndex());
				if (_location.back() != '/') {
					_location.insert(_location.end(), '/');
				}
				return false;
			}
		}
	}
	return true;
}

bool HttpResponse::buildBody() {
	if (_request.getBody().length() > std::stoul(_server.getClientMaxBodySize())) {
		_statusCode = HttpStatusCodes::PAYLOAD_TOO_LARGE;
		return false;
	}

	if (!handleTarget()) {
		return false;
	}

	if (_cgi || _autoIndex) {
		return true;
	}

	if (_statusCode != HttpStatusCodes::NONE && _statusCode != HttpStatusCodes::OK) {
		return true;
	}

	if (_request.getMethod() == Method::GET || _request.getMethod() == Method::HEAD) {
		if (!readFile()) {
			return false;
		}
	}
	else if (_request.getMethod() == Method::POST || _request.getMethod() == Method::PUT) {
		if (FileUtils::isFileExistAndReadable(_targetFile, "") && _request.getMethod() == Method::POST) {
			_statusCode = HttpStatusCodes::FORBIDDEN;
			return true;
		}

		std::ofstream fout(_targetFile, std::ios::binary);
		if (!fout) {
			_statusCode = HttpStatusCodes::FORBIDDEN;
			return false;
		}

		// TODO //TODO hier de moeder
		if (!_request.getBoundary().empty()) {
			std::string tempBody = _request.getBody();
			//tempBody = removeBoundary(tempBody, request.getBoundary());
			//file.write(body.c_str(), body.length());
		}
		else{
			//file.write(request.getBody().c_str(), request.getBody().length());
		}

	}
	else if (_request.getMethod() == Method::DELETE) {
		if (!FileUtils::isFileExistAndReadable(_targetFile, "")) {
			_statusCode = HttpStatusCodes::NOT_FOUND;
			return false;
		}
		if (remove(_targetFile.c_str()) != 0) {
			_statusCode = HttpStatusCodes::INTERNAL_SERVER_ERROR;
			return false;
		}
	}
	_statusCode = HttpStatusCodes::OK;
	return true;
}

bool HttpResponse::readFile() { // TODO remove debug
	DEBUG_PRINT(GREEN, "path " + _targetFile);
	std::ifstream fin(_targetFile, std::ios::binary | std::ios::ate);
	if (!fin) {
		_statusCode = HttpStatusCodes::NOT_FOUND;
		return false;
	}

	_responseBodyLength = static_cast<size_t>(fin.tellg());
	fin.seekg(0, std::ios::beg);

	_responseContent.resize(_responseBodyLength);
	
	if (!fin.read(reinterpret_cast<char*>(_responseContent.data()), _responseBodyLength)) {
		_statusCode = HttpStatusCodes::INTERNAL_SERVER_ERROR;
		return false;
	}

	/* DEBUG START */
	if (_targetFile.substr(_targetFile.find_last_of(".")) == ".html") {
		std::cout << YELLOW << "Response body:" << RESET << std::endl;
		for (char c : _responseContent) {
			std::cout << c;
		}
		std::cout << RESET << std::endl;
		std::cout << RESET << std::endl;
	}
	else {
		DEBUG_PRINT("Image size: " << _responseContent.size() << " bytes");
	}
	/* DEBUG END */
	return true;
}

HttpStatusCodes HttpResponse::getErrorCode() const{
	return _statusCode;
}

bool HttpResponse::requestIsSuccessful() {
	_statusCode = _request.errorCode();
	return _statusCode == HttpStatusCodes::NONE;
}

// first funtion to work on this is the heart of response.
void HttpResponse::buildResponse() {

	// if true handle error
	if (!requestIsSuccessful() || !buildBody()) {
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
	if (_request.getMethod() != Method::HEAD && (_request.getMethod() == Method::GET || _statusCode != HttpStatusCodes::OK)) {
		//_responseContent.append(_response_body);
	}
}