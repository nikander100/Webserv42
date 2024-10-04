#include "Request.hpp"

Request::Request() : 
	_state(Start), _method(Method::UNKNOWN), _statusCode(HTTP::StatusCode::Code::NONE), _serverName(""),
	_path(""), _query(""), _fragment(""), _verMajor(0), _verMinor(0), _contentLength(0),
	_chunkSize(0), _headers(), _body(""), _boundary(""), _multipartReadLength(0), _isMultipart(false), _isChunked(false) {
}

Request::~Request() {
}

bool Request::parsingComplete() const {
	return _state == Complete || _statusCode != HTTP::StatusCode::Code::NONE;
}
// can possibly be removed
// bool Request::parsingComplete() const {
//     return _flagRequestMethodAndHeaderDone && (!_flagBody || _flagBodyDone);
// }

// checks http uri according to RFC 3986 (Uniform Resource Identifier) -- this s not needed for this project but easy to implement
bool Request::isValidUri(const std::string &uri) {
	const std::string allowedChars = 
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789"
		"-._~:/?#[]@!$&'()*+,;=%"; // Added '%' to allowed characters

	for (size_t i = 0; i < uri.length(); ++i) {
		char c = uri[i];
		if (allowedChars.find(c) == std::string::npos) {
			// Check for percent-encoded characters
			if (c == '%' && i + 2 < uri.length() && 
				isxdigit(uri[i + 1]) && isxdigit(uri[i + 2])) {
				i += 2; // Skip the next two hex digits
			} else {
				return false;
			}
		}
	}

	return true;
}

std::string Request::decodeUri(const std::string &uri) {
	std::ostringstream decoded;
	for (size_t i = 0; i < uri.length(); ++i) {
		if (uri[i] == '%' && i + 2 < uri.length() && isxdigit(uri[i + 1]) && isxdigit(uri[i + 2])) {
			std::string hex = uri.substr(i + 1, 2);
			decoded << static_cast<char>(std::stoi(hex, nullptr, 16));
			i += 2;
		} else {
			decoded << uri[i];
		}
	}
	return decoded.str();
}

// checks http header according to RFC 2616 (HTTP/1.1) tokens are defined in RFC 2616 section 2.2
bool Request::isValidToken(const std::string &token) {
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
bool Request::parseRequestLine(const std::string &line) { // possibly rename to parserequest
	std::regex pattern(R"(^(\w+)\s+(([^?#]*)(\?[^#]*)?(#.*)?)\s+HTTP/(\d)\.(\d)\r?$)");
	std::smatch match;

	if (std::regex_match(line, match, pattern)) {
		_method = stringToMethod(match[1]);
		// validate path more like RFC for improved security. (not required for the project but easy to implement)
		if (!isValidUri(match[3])) {
			return false;
		}
		_path = decodeUri(match[3]);
		_query = match[4].length() > 1 ? match.str(4).substr(1) : ""; // remove leading '?'
		_fragment = match[5].length() > 1 ? match.str(5).substr(1) : ""; // remove leading '#'
		_verMajor = std::stoi(match[6]);
		_verMinor = std::stoi(match[7]);

		// Check the combined length of _path and _query
		if (_path.length() + _query.length() > MAX_URI_LENGTH) {
			_statusCode = HTTP::StatusCode::Code::URI_TOO_LONG;
			return false;
		}


		return _method != Method::UNKNOWN;
	}
	return false;
}

bool Request::parseHeader(const std::string &line) {
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
				DEBUG_PRINT("Invalid header key");
				return false;
			}

			// trim whitespaces from value
			// value.erase(0, value.find_first_not_of(' ')); // leading
			// value.erase(value.find_last_not_of(' ') + 1); // trailing

			_headers[key] = value;
			return true;
		}
	}
	return false;
}

bool Request::handleHeaders(std::istream& stream) {
	if (_method != Method::POST && _method != Method::PUT) {
		stream.ignore(1); // Skip \r\n
	}
	if (_headers.find("host") != _headers.end()) {
		_serverName = _headers["host"];
	} else {
		_statusCode = HTTP::StatusCode::Code::BAD_REQUEST;
		DEBUG_PRINT("Invalid request: Host header missing");
		return false;
	}
	if (_headers.find("transfer-encoding") != _headers.end() && _headers["transfer-encoding"] == "chunked") {
		if (_headers.find("content-length") != _headers.end()) {
			_statusCode = HTTP::StatusCode::Code::BAD_REQUEST;
			DEBUG_PRINT("Invalid request: both Transfer-Encoding and Content-Length headers present");
			return false;
		}
		_state = Reading_Chunk_Size;
		} else if (_headers.find("content-length") != _headers.end() || (_headers.find("content-type") != _headers.end() && _headers["content-type"].find("multipart/form-data") != std::string::npos)) {
		if (_headers.find("content-length") != _headers.end()) {
			try {
				_contentLength = std::stoul(_headers["content-length"]);
			} catch (const std::invalid_argument& ia) {
				DEBUG_PRINT("Invalid content-length: " << ia.what());
				return false;
			} catch (const std::out_of_range& oor) {
				DEBUG_PRINT("Content-length out of range: " << oor.what());
				return false;
			}
		}
		if (_headers.find("content-type") != _headers.end() && _headers["content-type"].find("multipart/form-data") != std::string::npos) {
			size_t pos = _headers["content-type"].find("boundary=");
			if (pos != std::string::npos) {
				_boundary = _headers["content-type"].substr(pos + 9);
				_isMultipart = true;
				_state = Reading_Multipart_Data;
				return true;
			} else {
				_statusCode = HTTP::StatusCode::Code::BAD_REQUEST;
				DEBUG_PRINT("Invalid request: multipart/form-data boundary missing");
				return false;
			}
		}
		_state = Reading_Body_Data; // Or a specific multipart state if needed
	} else {
		// check if request is malformed
		if (!stream.eof()) {
			_statusCode = HTTP::StatusCode::Code::BAD_REQUEST;
			DEBUG_PRINT("Malformed request");
			return false;
		}
		_state = Complete;
	}
	return true;
}

bool Request::parseChunkSize(const std::string &line) {
	std::istringstream hexStream(line);

	// parse input  >> skip whitespaces >> parse hex value >> store in _chunkSize
	hexStream >> std::ws >> std::hex >> _chunkSize;
	// Check if the entire stream was parsed correctly
	if (hexStream.fail() || !hexStream.eof()) {
		return false;
	}
	return true;
}

bool Request::handleTextData(std::istringstream &stream) {
	std::string line;
	if (!std::getline(stream, line)) {
		return true; // Exit if no more lines are available
	}

	// Re add the \n character
	if (!line.empty() && line.back() == '\r') {
		line.append("\n");
	}
	_multipartReadLength += line.size();

	// Check for boundary
	if (line == "--" + _boundary + "\r\n") {
		_body += line; // Add the boundary line to the body
		_multipartIsBinary = false;
	}
	// Check for end of multipart request
	else if (line == "--" + _boundary + "--\r\n") {
		_body += line; // Add the final boundary with \r\n
		_state = Complete;
	}
	// Handle (headers) text data
	else if (!line.empty()) {
		if (line == "\r\n") {
			_multipartIsBinary = true;
		}
		_body += line; // Add the line to the body
	}
	// Error handling
	else {
		_statusCode = HTTP::StatusCode::Code::BAD_REQUEST;
		_state = Complete;
		DEBUG_PRINT(RED, "Invalid multipart data");
		return false;
	}

	return true;
}

bool Request::handleBinaryData(std::istringstream &stream) {
	char buffer[4096];
	std::streampos initialPos = stream.tellg();

	stream.read(buffer, sizeof(buffer));
	std::streamsize bytesRead = stream.gcount();
	if (bytesRead > 0) {
		_multipartReadLength += bytesRead;

		// Check for the boundary in the binary data
		std::string bufferStr(buffer, bytesRead);
		size_t boundaryPos = bufferStr.find("--" + _boundary + "--\r\n");
		_body.append(buffer, bytesRead);
		if (boundaryPos != std::string::npos) {
			_multipartIsBinary = false;
			_state = Complete;
			return true;
		}

	}

	return true;
}

bool Request::parseMultipartData(std::istringstream &stream) {
	while (true) {
		if (_multipartIsBinary) {
			if (!handleBinaryData(stream)) {
				return false;
			}
		} else {
			if (!handleTextData(stream)) {
				return false;
			}
		}

		if (_multipartReadLength > _contentLength) {
			_statusCode = HTTP::StatusCode::Code::BAD_REQUEST;
			_state = Complete;
			DEBUG_PRINT(RED, "Error: Received more bytes than Content-Length!");
			return false;
		}

		if (stream.eof() && _state != Complete) {
			_state = Part_Complete;
			break;
		}
		if (_state == Complete) {
			break;
		}
	}

	if (_state == Complete && _multipartReadLength != _contentLength) {
		_statusCode = HTTP::StatusCode::Code::BAD_REQUEST;
		_state = Complete;
		DEBUG_PRINT(RED, "Error: Mismatch between received bytes and Content-Length!");
		DEBUG_PRINT(YELLOW, "Content-Length: " << _contentLength << ", Received bytes: " << _multipartReadLength);
		return false;
	}

	return true;
}


void Request::print() const {
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

bool Request::feed(const std::string &data) {
	std::istringstream stream(data);
	std::string line;

	while (_state != Complete) {
		
		switch (_state) {
			case Start: {
				if (!std::getline(stream, line)) {
					DEBUG_PRINT("Failed to read request line");
					return false;
					_statusCode = HTTP::StatusCode::Code::NOT_IMPLEMENTED;
				}

				if (parseRequestLine(line)) {
					_state = Method_Line_Parsed;
				} else {
					if (_statusCode == HTTP::StatusCode::Code::URI_TOO_LONG) {
						DEBUG_PRINT("URI too long");
					} else {
						_statusCode = HTTP::StatusCode::Code::BAD_REQUEST;
						DEBUG_PRINT("Invalid request line");
					}
					return false;
				}
				break;
			}
			case Method_Line_Parsed: {
				if (!std::getline(stream, line)) {
					_statusCode = HTTP::StatusCode::Code::BAD_REQUEST;
					DEBUG_PRINT("Failed to read request line");
					return false;
				}

				if (line == "\r") {
					_state = Header_Parsed;
				} else if (!parseHeader(line)) {
					_statusCode = HTTP::StatusCode::Code::BAD_REQUEST;
					DEBUG_PRINT("Invalid header");
					return false;
				}
				break;
			}
			case Header_Parsed: {
				if (!handleHeaders(stream)) {
					return false;
				}
				break;
			}
			case Reading_Body_Data: {
				std::streamoff currentPos = stream.tellg();
				std::streamoff dataSize = static_cast<std::streamoff>(data.size());
				if (_contentLength > 0 && currentPos + static_cast<std::streamoff>(_contentLength) <= dataSize) {
					_body = data.substr(currentPos, _contentLength);
					stream.seekg(_contentLength, std::ios::cur); // Move the get pointer forward
					_state = Complete;
				} else {
					_statusCode = HTTP::StatusCode::Code::BAD_REQUEST;
					DEBUG_PRINT("Invalid content-length or incomplete body");
					_state = Complete;
				}
				break;
			}
			case Reading_Chunk_Size: {
				if (!std::getline(stream, line)) {
					_statusCode = HTTP::StatusCode::Code::BAD_REQUEST;
					DEBUG_PRINT("Failed to read chunk size");
					return false;
				}
				if (!parseChunkSize(line)) {
					_statusCode = HTTP::StatusCode::Code::BAD_REQUEST;
					DEBUG_PRINT("Invalid chunk size");
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
					_state = Reading_Chunk_Size;
				} else {
					_statusCode = HTTP::StatusCode::Code::BAD_REQUEST;
					DEBUG_PRINT("Incomplete chunk data");
					return false;
				}
				break;
			}
			case Reading_Multipart_Data: {
				if (!parseMultipartData(stream)) {
					_statusCode = HTTP::StatusCode::Code::BAD_REQUEST;
					DEBUG_PRINT("Invalid multipart data");
					return false;
				}
				break;
			}
			case Part_Complete: {
				_state = Reading_Multipart_Data;
				return true;
			}
			default:
				_statusCode = HTTP::StatusCode::Code::BAD_REQUEST;
				DEBUG_PRINT("Unknown state");
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

const Method &Request::getMethod() const {
	return _method;
}

const std::string &Request::getServerName() const {
	return _serverName;
}

std::string &Request::getPath() {
	return _path;
}

std::string &Request::getQuery() {
	return _query;
}

const std::string &Request::getFragment() const {
	return _fragment;
}

const std::string &Request::getHeader(const std::string &key) const {
	auto it = _headers.find(key);
	if (it != _headers.end()) {
		return it->second;
	} else {
		static const std::string emptyString;
		return emptyString;
	}
}

std::unordered_map<std::string, std::string> Request::getHeaders() const {
	return _headers;
}

const std::string &Request::getBody() const {
	return _body;
}

const std::string &Request::getBoundary() const {
	return _boundary;
}

HTTP::StatusCode::Code Request::errorCode() const {
	return _statusCode;
}

bool Request::keepAlive() const {
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

void Request::reset() {
	// Only reset if the request is not multipart or chunked, or if it is complete
	if (_state == Complete) {
		_state = Start;
		_method = Method::UNKNOWN;
		_statusCode = HTTP::StatusCode::Code::NONE;
		_serverName.clear();
		_path.clear();
		_query.clear();
		_fragment.clear();
		_verMajor = 0;
		_verMinor = 0;
		_contentLength = 0;
		_chunkSize = 0;
		_headers.clear();
		_body.clear();
		_boundary.clear();
		_isMultipart = false; // Reset the multipart flag
		_isChunked = false; // Reset the chunked flag
		_multipartReadLength = 0; // Reset the multipart read length
	}
}