#include "../includes/HttpRequest.hpp"

HttpRequest::HttpRequest() : _state(Start), _method(Method::UNKNOWN), _errorCode(HttpStatusCodes::NONE), _verMajor(0), _verMinor(0),
_contentLength(0), _chunkSize(0), _headers(), _body() {
}

HttpRequest::~HttpRequest() {
}

bool HttpRequest::parsingComplete() const {
	return _state == Complete;
}
// can possibly be removed
// bool HttpRequest::parsingComplete() const {
//     return _flagRequestMethodAndHeaderDone && (!_flagBody || _flagBodyDone);
// }

// checks http uri according to RFC 3986 (Uniform Resource Identifier) -- this s not needed for this project but easy to implement
bool HttpRequest::isValidUri(const std::string &uri) {
	const std::string allowedChars = 
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789"
		"-._~:/?#[]@!$&'()*+,;=";

	for (char c : uri) {
		if (allowedChars.find(c) == std::string::npos) {
			return false;
		}
	}

	return true;
}

// checks http header according to RFC 2616 (HTTP/1.1) tokens are defined in RFC 2616 section 2.2
bool HttpRequest::isValidToken(const std::string &token) {
	const std::string allowedChars = 
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789"
		"!#$%&'*+-.^_`|~";

	for (char c : token) {
		if (allowedChars.find(c) == std::string::npos) {
			return false;
		}
	}

	return true;
}

//currently up to standard RFC1945
//new uri is RFC3986 but we only have to comply with the RFC for 
//http1.0/1.1
bool HttpRequest::parseRequestLine(const std::string &line) { // possibly rename to parserequest
	std::regex pattern(R"(^(\w+)\s+(([^?#]*)(\?[^#]*)?(#.*)?)\s+HTTP/(\d)\.(\d)\r?$)");
	std::smatch match;

	if (std::regex_match(line, match, pattern)) {
		_method = stringToMethod(match[1]);
		_path = match[3];
		_query = match[4].length() > 1 ? match.str(4).substr(1) : ""; // remove leading '?'
		_fragment = match[5].length() > 1 ? match.str(5).substr(1) : ""; // remove leading '#'
		_verMajor = std::stoi(match[6]);
		_verMinor = std::stoi(match[7]);

		// Check the combined length of _path and _query
		if (_path.length() + _query.length() > MAX_URI_LENGTH) {
			_errorCode = HttpStatusCodes::URI_TOO_LONG;
			return false;
		}

		// validate path more like RFC for improved security. (not required for the project but easy to implement)
		if (!isValidUri(_path)) {
			return false;
		}
		return _method != Method::UNKNOWN;
	}
	return false;
}

bool HttpRequest::parseHeader(const std::string &line) {
	// Check if the string ends with '\r'
	if (!line.empty() && line.back() == '\r') {
		// Remove the '\r' character from the end of the string
		std::string temp = line.substr(0, line.size() - 1);
		// Now continue with parsing the modified string
		// Use the modified string for matching
		std::regex pattern(R"(\s*([^:\s]+):\s*([^\r]+)\s*)");
		std::smatch match;
		
		if (std::regex_match(temp, match, pattern)) {
			std::string key = match[1];
			std::string value = match[2];

			// convert key to lowercase
			std::transform(key.begin(), key.end(), key.begin(), ::tolower);

			// validate key
			if (!isValidToken(key)) {
				std::cerr << "Invalid header key" << std::endl;
				return false;
			}

			// trim whitespaces from value
			// value.erase(0, value.find_first_not_of(' ')); // leading
			// value.erase(value.find_last_not_of(' ') + 1); // trailing

			_headers[key] = value;

			// possibly move this to a separate function/state..
			if (key == "content-length") {
				try {
					_contentLength = std::stoul(value);
				} catch (const std::invalid_argument& ia) {
					std::cerr << "Invalid content-length: " << ia.what() << std::endl;
					return false;
				} catch (const std::out_of_range& oor) {
					std::cerr << "Content-length out of range: " << oor.what() << std::endl;
					return false;
				}
			}
			return true;
		}
	}
	return false;
}

bool HttpRequest::parseChunkSize(const std::string &line) {
	std::istringstream hexStream(line);

	// parse input  >> skip whitespaces >> parse hex value >> store in _chunkSize
	hexStream >> std::ws >> std::hex >> _chunkSize;
	// Check if the entire stream was parsed correctly
	if (hexStream.fail() || !hexStream.eof()) {
		return false;
	}
	return true;
}

void HttpRequest::print() const {
	std::cout << "Method: " << methodToString(_method) << "\n";
	std::cout << "Path: " << _path << "\n";
	std::cout << "Query: " << _query << "\n";
	std::cout << "Fragment: " << _fragment << "\n";
	std::cout << "HTTP Version: " << static_cast<int>(_verMajor) << "." << static_cast<int>(_verMinor) << "\n";
	std::cout << "Headers:\n";
	for (const auto& header : _headers) {
		std::cout << header.first << ": " << header.second << "\n";
	}
	std::cout << "Body:\n" << _body << "\n";
}

bool HttpRequest::feed(const std::string &data) {
	std::istringstream stream(data);
	std::string line;

	while (_state != Complete) {
		
		switch (_state) {
			case Start: {
				if (!std::getline(stream, line)) {
					std::cerr << "Failed to read request line" << std::endl;
					return false;
					_errorCode = HttpStatusCodes::NOT_IMPLEMENTED;
				}

				if (parseRequestLine(line)) {
					_state = Method_Line_Parsed;
				} else {
					if (_errorCode == HttpStatusCodes::URI_TOO_LONG) {
						std::cerr << "URI too long" << std::endl;
					} else {
						_errorCode = HttpStatusCodes::BAD_REQUEST;
						std::cerr << "Invalid request line" << std::endl;
					}
					return false;
				}
				break;
			}
			case Method_Line_Parsed: {
				if (!std::getline(stream, line)) {
					_errorCode = HttpStatusCodes::BAD_REQUEST;
					std::cerr << "Failed to read request line" << std::endl;
					return false;
				}

				if (line == "\r") {
					_state = Header_Parsed;
				} else if (!parseHeader(line)) {
					_errorCode = HttpStatusCodes::BAD_REQUEST;
					std::cerr << "Invalid header" << std::endl;
					return false;
				}
				break;
			}
			case Header_Parsed: {
				stream.ignore(1); // Skip \r\n
				if (_headers.find("transfer-encoding") != _headers.end() && _headers["transfer-encoding"] == "chunked") {
					// _flagBody = true;
					if (_headers.find("content-length") != _headers.end()) {
						_errorCode = HttpStatusCodes::BAD_REQUEST;
						std::cerr << "Invalid request: both Transfer-Encoding and Content-Length headers present" << std::endl;
						return false;
					}
					_state = Reading_Chunk_Size;
				} else if (_contentLength > 0) {
					// _flagBody = true;
					_state = Reading_Body_Data;
				} else {
					// check if request is malformed
					if (!stream.eof()) {
						_errorCode = HttpStatusCodes::BAD_REQUEST;
						std::cerr << "Malformed request" << std::endl;
						return false;
					}
					_state = Complete;
				}
				break;
			}
			case Reading_Body_Data: {
				if (_contentLength > 0 && stream.tellg() + static_cast<std::streamoff>(_contentLength) <= static_cast<std::streamoff>(data.size())) {
					_body = data.substr(stream.tellg(), _contentLength);
					stream.seekg(_contentLength, std::ios::cur); // Move the get pointer forward
					// _flagBodyDone = true;
					_state = Complete;
				} else {
					_errorCode = HttpStatusCodes::BAD_REQUEST;
					std::cerr << "Invalid content-length or incomplete body" << std::endl;
					_state = Complete;
				}
				break;
			}
			case Reading_Chunk_Size: {
				if (!std::getline(stream, line)) {
					_errorCode = HttpStatusCodes::BAD_REQUEST;
					std::cerr << "Failed to read chunk size" << std::endl;
					return false;
				}
				if (!parseChunkSize(line)) {
					_errorCode = HttpStatusCodes::BAD_REQUEST;
					std::cerr << "Invalid chunk size" << std::endl;
					return false;
				}
				if (_chunkSize == 0) {
					_state = Complete;
				} else {
					_state = Reading_Chunk_Data;
				}
				break;
			}
			case Reading_Chunk_Data: {
				if (stream.tellg() + static_cast<std::streamoff>(_chunkSize + 2) <= static_cast<std::streamoff>(data.size())) {
					_body.append(data.substr(stream.tellg(), _chunkSize));
					stream.seekg(_chunkSize, std::ios::cur); // Move the get pointer forward
					stream.ignore(2); // Skip trailing \r\n
					// _flagBodyDone = true;
					_state = Reading_Chunk_Size;
				} else {
					_errorCode = HttpStatusCodes::BAD_REQUEST;
					std::cerr << "Incomplete chunk data" << std::endl;
					return false;
				}
				break;
			}
			default:
				std::cerr << "Unkown state" << std::endl;
				_errorCode = HttpStatusCodes::BAD_REQUEST;
				return false;
				// instead of returning set error flag so later can reset the request object.
					// send bad ... response.

		}
		if (_state == Complete) {
			break;
		}
	}
	return true;
}

const Method &HttpRequest::getMethod() const {
	return _method;
}

const std::string &HttpRequest::getPath() const {
	return _path;
}

std::string &HttpRequest::getQuery() {
	return _query;
}

const std::string &HttpRequest::getFragment() const {
	return _fragment;
}

const std::string &HttpRequest::getHeader(const std::string &key) const {
	return _headers.at(key); // TODO: possibly change this to find and return empty string if not found.
}
// const std::string &HttpRequest::getHeader(const std::string &key) const {
// 	auto it = _headers.find(key);
// 	if (it != _headers.end()) {
// 		return it->second;
// 	} else {
// 		static const std::string emptyString;
// 		return emptyString;
// 	}
// }

std::unordered_map<std::string, std::string> HttpRequest::getHeaders() const {
	return _headers;
}

const std::string &HttpRequest::getBody() const {
	return _body;
}

HttpStatusCodes HttpRequest::errorCode() const {
	return _errorCode;
}

bool HttpRequest::keepAlive() const {
	if (_verMajor == 1 && _verMinor == 1) {
		if (_headers.find("connection") != _headers.end()) {
			if (_headers.at("connection") == "close") {
				return false;
			}
			return _headers.at("connection") == "keep-alive";
		}
		return true;
	} else if (_verMajor == 1 && _verMinor == 0) {
		if (_headers.find("connection") != _headers.end()) {
			return _headers.at("connection") != "close";
		}
		return false;
	}
	return false;
}

void HttpRequest::reset() {
	_state = Start;
	_method = Method::UNKNOWN;
	_errorCode = HttpStatusCodes::NONE;
	_path.clear();
	_query.clear();
	_fragment.clear();
	_verMajor = 0;
	_verMinor = 0;
	_contentLength = 0;
	_chunkSize = 0;
	_headers.clear();
	_body.clear();
}