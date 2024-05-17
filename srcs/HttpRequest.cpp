#include "HttpRequest.hpp"

HttpRequest::HttpRequest()
	: _path(""), _state(Request_Line), _wordIndex(0), _lineNumber(0),
	_completeFlag(false), _skip(false), _storage(""), _keyStorage(""),
	_methodIndex(1)
{
	_methodString[HttpMethod::GET] = "GET";
	_methodString[HttpMethod::POST] = "POST";
	_methodString[HttpMethod::DELETE] = "DELETE";
}

HttpRequest::~HttpRequest() {
}

// check docs.md on info about uri.
bool allowedUri(uint8_t ch) {
	static std::set<uint8_t> allowedChars = {
		'&', '\'', '(', ')', '*', '+', ',', '-', '.', '/', '0', '1', '2', '3', '4',
		'5', '6', '7', '8', '9', ':', ';', '?', '@', '[', '!', '#', '$', '=', '_',
		'~'
	};

	// Add all the uppercase letters to the allowed characters
	for (char c = 'A'; c <= 'Z'; ++c) {
		allowedChars.insert(c);
	}

	// Add all the lowercase letters to the allowed characters
	for (char c = 'a'; c <= 'z'; ++c) {
		allowedChars.insert(c);
	}

	return allowedChars.count(ch) > 0;
}


/* void HttpRequest::feed(const std::string& data) {
	for(const auto& character : data){
		if(_skip) {
			_skip = false;
			continue;
		}

		switch(_state)
		{
			case Request_Line:
				handleRequestLine(character);
				break;
			case Request_Line_Method:
				handleRequestMethod(character);
				break;
			case Request_Line_First_Space:
				handleRequestFirstSpace(character);
				break;
			case Request_Line_Before_URI:
				handleRequestBeforeURI(character);
				break;
			// Add other cases here...
			default:
				std::cout << "Invalid state" << std::endl;
				return;
		}
	}
}

void HttpRequest::handleRequestLine(const char& character) {
	if (character == 'G')
		_method = GET;
	else if (character == 'P')
		_method = POST;
	else if (character == 'D')
		_method = DELETE;
	else
	{
		std::cout << "Wrong Method" << std::endl; 
		return;
	}
	_state = Request_Line_Method;
} */


// TODO convert funciton below to the one in the comment above, test if it works, if it does then possibly convert that to regex instead of a finite state machine.




// Define other handler functions here...


/**
 * @brief Feeds the HTTP request with data.
 * 
 * This function processes the provided data and updates the state of the HTTP request object accordingly.
 * This is a state machine that processes the data one byte at a time.
 * @param data A pointer to the data to be processed.
 * @param size The size of the data to be processed.
 */
void HttpRequest::feed(char* data, size_t size) {
	u_int8_t character;

	for (size_t i = 0; i < size; ++i) {
		character = data[i];
		if (_skip) {
			_skip = false;
			continue;
		}

		switch (_state) {
			case Request_Line:
				switch (character) {
					case 'G': _method = GET; break;
					case 'P': _method = POST; break;
					case 'D': _method = DELETE; break;
					default: std::cout << "Wrong Method" << std::endl; return;
				}
				_state = Request_Line_Method;
				break;

			case Request_Line_Method:
				if (character == _methodString[_method][_methodIndex])
					_methodIndex++;
				if (static_cast<std::string::size_type>(_methodIndex) == _methodString[_method].length())
					_state = Request_Line_First_Space;
				break;

			case Request_Line_First_Space:
				if (character != ' ') {
					std::cout << "Bad Character (First_Space)" << std::endl; return;
				}
				_state = Request_Line_Before_URI;
				break;

			case Request_Line_Before_URI:
				if (character == ' ' || character == '\r' || character == '\n' || !allowedUri(character)) {
					std::cout << "Bad Character (Before_URI)" << std::endl; return;
				}
				_storage.clear();
				_state = Request_Line_URI;
				break;

			case Request_Line_URI:
				if (character == ' ') {
					_path.append(_storage);
					_storage.clear();
					_state = Request_Line_Ver;
				} else if (!allowedUri(character)) {
					std::cout << "Bad Character (URI)" << std::endl; return;
				} else {
					_storage += character;
				}
				break;

			case Request_Line_Ver:
				if (character != 'H') {
					std::cout << "Bad Version" << std::endl; return;
				}
				_state = Request_Line_HT;
				break;

			case Request_Line_HT:
				if (character != 'T') {
					std::cout << "Bad Version" << std::endl; return;
				}
				_state = Request_Line_HTT;
				break;

			case Request_Line_HTT:
				if (character != 'T') {
					std::cout << "Bad Version" << std::endl; return;
				}
				_state = Request_Line_HTTP;
				break;

			case Request_Line_HTTP:
				if (character != 'P') {
					std::cout << "Bad Version" << std::endl; return;
				}
				_state = Request_Line_HTTP_Slash;
				break;

			case Request_Line_HTTP_Slash:
				if (character != '/') {
					std::cout << "Bad Version(Slash)" << std::endl; return;
				}
				_state = Request_Line_Major;
				break;

			case Request_Line_Major:
				if (!isdigit(character)) {
					std::cout << "Bad Version(Major)" << std::endl; return;
				}
				_verMajor = character - '0';
				_state = Request_Line_Dot;
				break;

			case Request_Line_Dot:
				if (character != '.') {
					std::cout << "Bad Version(Dot)" << std::endl; return;
				}
				_state = Request_Line_Minor;
				break;

			case Request_Line_Minor:
				if (!isdigit(character)) {
					std::cout << "Bad Version(Minor)" << std::endl; return;
				}
				_verMinor = character - '0';
				_state = Request_Line_CR;
				break;

			case Request_Line_CR:
				if (character != '\r') {
					std::cout << "Bad Character(Request_Line_CR)" << std::endl; return;
				}
				_state = Request_Line_LF;
				break;

			case Request_Line_LF:
				if (character != '\n') {
					std::cout << "Bad Character(Request_Line_LF)" << std::endl; return;
				}
				_state = H_Key;
				break;

			case H_Key:
				if (character == ':') {
					_keyStorage = _storage;
					_storage.clear();
					_state = H_Value;
					continue;
				} else if (character == '\r') {
					_completeFlag = true;
					break;
				}
				break;

			case H_Value:
				if (character == '\r') {
					setHeader(_keyStorage, _storage);
					_keyStorage.clear();
					_storage.clear();
					_skip = true;
					_state = H_Key;
					continue;
				}
				break;

			default:
				std::cout << "Invalid state" << std::endl;
				return;
		}

		_storage += character;
	}
}

bool HttpRequest::parsingCompleted() {
	return _completeFlag;
}

HttpMethod &HttpRequest::getMethod() {
	return _method;
}
void HttpRequest::setMethod(HttpMethod & method) {
	_method = method;
}

std::string &HttpRequest::getHeader(std::string &name){
	return _requestHeaders[name];
}
void HttpRequest::setHeader(std::string name, std::string value) {
	_requestHeaders[name] = value;
}

void HttpRequest::printMessage()
{
	std::cout << _methodString[_method] + " " + _path + " " + "HTTP/" << _verMajor << "." << _verMinor << '\n';

	for(const auto& header : _requestHeaders)
	{
		std::cout << header.first << ":" << header.second << '\n';
	}
}