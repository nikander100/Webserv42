#include "HttpRequest.hpp"

HttpRequest::HttpRequest() : _state(Start), _method(UNKNOWN), _verMajor(0), _verMinor(0),
_contentLength(0), _chunkSize(0) {
}

HttpRequest::~HttpRequest() {
}

Method HttpRequest::parseMethod(const std::string &method) {
	if (method == "GET") return GET;
	if (method == "POST") return POST;
	if (method == "DELETE") return DELETE;
	return UNKNOWN;
}

bool HttpRequest::parseRequestLine(const std::string &line) {
	std::regex pattern(R"(^(\w+)\s+(\S+)\s+HTTP/(\d)\.(\d)$)");
	std::smatch match;

	if (std::regex_match(line, match, pattern)) {
		_method = parseMethod(match[1]);
		_path = match[2];
		_verMajor = std::stoi(match[3]);
		_verMinor = std::stoi(match[4]);
		return _method != UNKNOWN;
	}
	return false;
}

bool HttpRequest::parseHeader(const std::string &line) {
	std::regex pattern(R"(^([^:]+):\s*(.+)$)");
	std::smatch match;

	if (std::regex_match(line, match, pattern)) {
		_headers[match[1]] = match[2];
		if (match[1] == "Content-Length") {
			_contentLength = std::stoul(match[2]);
		}
		return true;
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

// void HttpRequest::parseBodyData(const std::string& data, size_t& pos) {
// 	if (_contentLength > 0 && pos + _contentLength <= data.size()) {
// 		_body = data.substr(pos, _contentLength);
// 		pos += _contentLength;
// 		_state = Complete;
// 	} else {
// 		std::cerr << "Invalid Content-Length or incomplete body" << std::endl;
// 		_state = Complete;
// 	}
// }


bool HttpRequest::feed(const std::string &data) {
	std::istringstream stream(data);
	std::string line;
	size_t pos = 0;

	if (_state == Start) {
		if (std::getline(stream, line) && parseRequestLine(line)) {
			_state = Request_Line_Parsed;
		} else {
			std::cerr << "Invalid request line" << std::endl;
			return false;
		}
	}

	while (std::getline(stream, line) && line != "\r") {
		pos += line.size() + 1; // +1 for the newline character
		if (_state == Request_Line_Parsed && parseHeader(line)) {
			_state = Header_Parsed;
		} else {
			std::cerr << "Invalid header" << std::endl;
			return false;
		}
	}

	if (_state == Header_Parsed) {
		pos += 2; // Skip the \r\n after the headers
		if (_headers.find("Transfer-Encoding") != _headers.end() && _headers["Transfer-Encoding"] == "chunked") {
			_state = Reading_Chunk_Size;
		} else if (_contentLength > 0) {
			_state = Reading_Body_Data;
		} else {
			_state = Complete;
		}
	}

	if (_state == Reading_Body_Data) {
		// parseBodyData(data, pos);
		if (_contentLength > 0 && pos + _contentLength <= data.size()) {
		_body = data.substr(pos, _contentLength);
		pos += _contentLength;
		_state = Complete;
	} else {
		std::cerr << "Invalid Content-Length or incomplete body" << std::endl;
		_state = Complete;
		}
	}

	while (_state == Reading_Chunk_Size || _state == Reading_Chunk_Data) {
		if (_state == Reading_Chunk_Size) {
			std::getline(stream, line);
			pos += line.size() + 1; // +1 for the newline character
			if (!parseChunkSize(line)) {
				std::cerr << "Invalid chunk size" << std::endl;
				return false;
			}
			if (_chunkSize == 0) {
				_state = Complete;
				break;
			}
			_state = Reading_Chunk_Data;
		}
	}

	if (_state == Reading_Chunk_Data) {
		parseChunkData(data, pos);
		_state = Reading_Chunk_Size;
	}

	return true;
}
