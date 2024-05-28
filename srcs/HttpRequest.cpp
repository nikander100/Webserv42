#include "../includes/HttpRequest.hpp"

// bool HttpRequest::isValidUri(const std::string &uri) {
//     const std::string allowedChars = 
//         "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
//         "abcdefghijklmnopqrstuvwxyz"
//         "0123456789"
//         "-._~:/?#[]@!$&'()*+,;=";

//     for (char c : uri) {
//         if (allowedChars.find(c) == std::string::npos) {
//             return false;
//         }
//     }

//     return true;
// }

HttpRequest::HttpRequest() : _state(Start), _method(UNKNOWN), _verMajor(0), _verMinor(0),
_contentLength(0), _chunkSize(0) {
}

HttpRequest::~HttpRequest() {
}

bool HttpRequest::parsingComplete() const {
	return _state == Complete;
}

Method HttpRequest::parseMethod(const std::string &method) {
	if (method == "GET") return GET;
	if (method == "POST") return POST;
	if (method == "DELETE") return DELETE;
	return UNKNOWN;
}


bool HttpRequest::parseMethodLine(const std::string &line) {
	std::regex pattern(R"(^(\w+)\s+(([^?#]*)(\?[^#]*)?(#.*)?)\s+HTTP/(\d)\.(\d)\r?$)");
	std::smatch match;

	if (std::regex_match(line, match, pattern)) {
		_method = parseMethod(match[1]);
		_path = match[3];
		_query = match[4].length() > 1 ? match.str(4).substr(1) : ""; // remove leading '?'
		_fragment = match[5].length() > 1 ? match.str(5).substr(1) : ""; // remove leading '#'
		_verMajor = std::stoi(match[6]);
		_verMinor = std::stoi(match[7]);

		// TODO: validate path. as regex already checks query and fragment.
		return _method != UNKNOWN;
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
		std::regex pattern(R"(([^:]+):\s*([^\r]+))");
		std::smatch match;
		
		if (std::regex_match(temp, match, pattern)) {
			_headers[match[1]] = match[2];
			if (match[1] == "Content-Length") {
				_contentLength = std::stoul(match[2]);
			}
			return true;
		}
	}
	return false;
}

bool HttpRequest::parseChunkSize(const std::string &line) {
	std::istringstream hexStream(line);
	hexStream >> std::hex >> _chunkSize;
	return !hexStream.fail();
}

void HttpRequest::parseChunkData(const std::string& data, size_t& pos) {
	if (_chunkSize > 0) {
		_body.append(data.substr(pos, _chunkSize));
		pos += _chunkSize + 2; // Skip chunk data and trailing \r\n
	}
}

void HttpRequest::print() const {
	std::cout << "Method: ";
	switch (_method) {
		case GET: std::cout << "GET"; break;
		case POST: std::cout << "POST"; break;
		case DELETE: std::cout << "DELETE"; break;
		default: std::cout << "UNKNOWN"; break;
	}
	std::cout << "\nPath: " << _path << "\n";
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
	size_t pos = 0;

	while (_state != Complete) {
		
		switch (_state) {
			case Start:
				if (!std::getline(stream, line)) {
					std::cerr << "Failed to read request line" << std::endl;
					return false;
					// instead of returning set error flag so later can reset the request object.
				}
				pos += line.size() + 1; // +1 for the newline character

				if (parseMethodLine(line)) {
					_state = Method_Line_Parsed;
				} else {
					std::cerr << "Invalid request line" << std::endl;
					// instead of returning set error flag so later can reset the request object.
					// send bad method response.
					return false;
				}
				break;

			case Method_Line_Parsed:
				if (!std::getline(stream, line)) {
					std::cerr << "Failed to read request line" << std::endl;
					return false;
					// instead of returning set error flag so later can reset the request object.
				}
				pos += line.size() + 1; // +1 for the newline character

				if (line == "\r") {
					_state = Header_Parsed;
				} else if (!parseHeader(line)) {
					std::cerr << "Invalid header" << std::endl;
					// instead of returning set error flag so later can reset the request object.
					// send bad method response.
					return false;
				}
				break;

			case Header_Parsed:
				pos += 1; // Skip the \r\n after the headers
				if (_headers.find("Transfer-Encoding") != _headers.end() && _headers["Transfer-Encoding"] == "chunked") {
					_state = Reading_Chunk_Size;
				} else if (_contentLength > 0) {
					_state = Reading_Body_Data;
				} else {
					_state = Complete;
				}
				break;

			case Reading_Body_Data:
				if (_contentLength > 0 && pos + _contentLength <= data.size()) {
					_body = data.substr(pos, _contentLength);
					pos += _contentLength;
					_state = Complete;
				} else {
					std::cerr << "Invalid Content-Length or incomplete body" << std::endl;
					_state = Complete;
				}
				break;
			
			case Reading_Chunk_Size:
				if (!parseChunkSize(line)) {
					std::cerr << "Invalid chunk size" << std::endl;
					return false;
				}
				if (_chunkSize == 0) {
					_state = Complete;
				} else {
					_state = Reading_Chunk_Data;
				}
				break;

			case Reading_Chunk_Data:
				parseChunkData(data, pos);
				_state = Reading_Chunk_Size;
				break;

			default:
				std::cerr << "Unkown state" << std::endl;
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
