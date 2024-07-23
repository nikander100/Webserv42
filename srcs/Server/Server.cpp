#include "Server.hpp"

// use _host(inet_addr(inaddrloopback)) to test on 127.0.0.1 and _host(inaddrany) to test on any ip. and inet_addr("10.11.4.1") to use local ip, 10.pc.row.floor
Server::Server() : _serverName(""), _port(TEST_PORT), _host(INADDR_ANY), _root(""),
	_clientMaxBodySize(MAX_CONTENT_SIZE), _index(""), _autoIndex(false) {
}

Server::~Server() {
	_socket.close();
}

// Server Block: server { ... }
// Takes a std::string from the parser should be:
// [server_name localhost;]
void Server::setServerName(std::string &server_name) {
	checkInput(server_name);
	_serverName = server_name;
}

std::string Server::getServerName() const {
	return _serverName;
}

// Server Block: server { ... }
// Takes a std::string from the parser should be:
// [host 127.0.0.1;]
void Server::setHost(std::string &host) {
	checkInput(host);
	if (host == "localhost") {
		_host = inet_addr("127.0.0.1");
	} else {
		struct sockaddr_in sockaddr;
		int result = inet_pton(AF_INET, host.c_str(), &(sockaddr.sin_addr));
		if (!result) {
			throw Error("Wrong syntax: host");
		}
		_host = inet_addr(host.c_str());
	}
}

std::string Server::getHost() const {
	struct in_addr ip_addr;
	ip_addr.s_addr = _host;
	return inet_ntoa(ip_addr);
}

// Server Block: server { ... }
// Takes a std::string from the parser should be:
// [listen 8002;]
void Server::setPort(std::string &port_string) {
	checkInput(port_string);

	// Check if the parameter is a valid number
	std::regex r("\\d+");
	if (!std::regex_match(port_string, r)) {
		throw Error("Wrong syntax: port");
	}

	// Check if the port is within the valid range
	int port = std::stoi(port_string);
	if (port < 1 || port > 65535)
		throw Error("Wrong syntax: port");
	// Convert the parameter to an 16bit integer

	_port = static_cast<uint16_t>(port);
}

std::string Server::getPort() const {
	return std::to_string(_port);
}

// Server Block: server { ... }
// Takes a std::string from the parser should be:
// [root docs/fusion_web/;]
void Server::setRoot(std::string &root) {
	checkInput(root);
	if (FileUtils::getTypePath(root) == FileType::DIRECTORY) {
		_root = root;
		return ;
	}

	std::string dir = std::filesystem::current_path();
	if (dir.empty()) {
		throw Error("Error getting current working directory");
	}
	
	std::string fullPath = dir + root;
	if (FileUtils::getTypePath(fullPath) != FileType::DIRECTORY) {
		// If not a directory, throw an exception
		throw std::runtime_error("Path is not a directory: " + fullPath);
	}
	_root = fullPath;
}

std::string Server::getRoot() const {
	return _root;
}

// Server Block: server { ... }
// Takes a std::string from the parser should be:
// [client_max_body_size 3000000;]
void Server::setClientMaxBodySize(std::string &client_max_body_size) {
	checkInput(client_max_body_size);
	try {
		_clientMaxBodySize = std::stoul(client_max_body_size);
	} catch (const std::invalid_argument &e){
		throw Error("Wrong syntax: clientMaxBodySize");
	} catch (std::out_of_range &e) {
		throw Error("Wrong syntax: clientMaxBodySize");
	}
}

std::string Server::getClientMaxBodySize() const {
	return std::to_string(_clientMaxBodySize);
}

// Server Block: server { ... }
// Takes a std::string from the parser should be:
// [index index.html;]
void Server::setIndex(std::string &index) {
	checkInput(index);
	_index = index;
}

std::string Server::getIndex() const {
	return _index;
}

// Server Block: server { ... }
// Takes a std::string from the parser should be:
// [autoindex on;]
void Server::setAutoIndex(std::string& autoindex) {
	checkInput(autoindex);

	if (autoindex != "on" && autoindex != "off") {
		throw std::runtime_error("Wrong syntax: autoindex must be either 'on' or 'off'");
	}

	_autoIndex = (autoindex == "on") ? true : false;
}

std::string Server::getAutoIndex() const {
	return _autoIndex ? "on" : "off";
}

int Server::getListenFd() const {
	return _socket.getFd();
}

const sockaddr_in Server::getServerAddress() const {
	return _socket.getAddress();
}
// NOTE:: errorpage is not part of location
// Server Block: server { ... }
// Takes a Vector of strings std::vector<std::string> from the parser should be:
// std::vector<std::string> {
// [error_page 404 error_pages/404.html;]
// [error_page 406 error_pages/406.html;]
// }
void Server::setErrorPages(const std::vector<std::string> &error_pages) {
	for (const std::string &page : error_pages) {
		std::istringstream iss(page);
		std::string error_keyword, status_code, path;
		if (!(iss >> error_keyword >> status_code >> path)) {
			throw Error("Invalid error page format: " + page);
		}
		if (error_keyword != "error_page") {
			throw Error("Invalid keyword in error page: " + page);
		}
		HttpStatusCodes code = static_cast<HttpStatusCodes>(std::stoi(status_code));
		setErrorPage(code, path);
	}
}

// set a specific error page
void Server::setErrorPage(HttpStatusCodes key, std::string path) {
	if (key < HttpStatusCodes::CONTINUE || key > HttpStatusCodes::NETWORK_AUTHENTICATION_REQUIRED) { // possibly check to 600 but there are no used codes above 511
		throw std::invalid_argument("Invalid HTTP status code");
	}

	//check if file exists
	if (FileUtils::getTypePath(path) != FileType::DIRECTORY) {
		if (FileUtils::getTypePath(_root + path) != FileType::FILE) {
			throw std::invalid_argument("Incorrect path for error page file: " + _root + path);
		}
		if (FileUtils::checkFile(_root + path) == -1) {
			throw std::invalid_argument("Error page file :" + _root + path + " is not accessible");
		}
	}

	_errorPages[key] = path;

}

const std::unordered_map<HttpStatusCodes, std::string> &Server::getErrorPages() const {
	return _errorPages;
}


// auto [isInternal, page] = server.getErrorPage(statusCode);
// if (isInternal) {
//     // Handle internal page
// } else {
//     // Handle custom page
// }
// returns either the path to the custom error page or the default internal error page
std::pair<bool, std::string> Server::getErrorPage(HttpStatusCodes key) {
	// Check if a custom error page has been set for this status code
	if (_errorPages.count(key) > 0) {
		return {false, _errorPages.at(key)};
	}

	// If not, check if the status code has an internal page in BuiltinErrorPages.hpp
	if (BuiltinErrorPages::isInternalPage(key)) {
		return {true, BuiltinErrorPages::getInternalPage(key)};
	}
	
	throw std::invalid_argument("Error page not found for status code: " + std::to_string(static_cast<int>(key)));
}


// location /subfolder {
//        root ./;
//        allow_methods GET POST DELETE;
//	  autoindex off;
//        index time.py;
//        cgi_path /usr/bin/python3 /bin/bash;
//        cgi_ext .py .sh;
//        return abc/index1.html;
//        alias google.com/;
//        client_max_body_size 1024;
//    }

// Location Block: location { ... }
// Takes a std::String and Vector of strings std::vector<std::string> from the parser should be:
// std:string path[/subfolder] std::vector<std::string> {
// [root ./;]
// [allow_methods GET POST DELETE;]
// [autoindex off;]
// [index time.py;]
// [cgi_path /usr/bin/python3 /bin/bash;]
// [cgi_ext .py .sh;]
// [return abc/index1.html;]
// [alias google.com/;]
// [client_max_body_size 1024;]
// }
void Server::setLocation(const std::string &path, std::vector<std::string> &parsedLocation) {
	Location newLocation;
	std::vector<Method> methods;
	std::vector<std::pair<std::string, std::string>> cgiPathExtension;

	std::regex rootRegex(R"(root\s(.+);)");
	std::regex methodRegex(R"(allow_methods\s+((GET|POST|PUT|HEAD|DELETE)\s*)+;)");
	std::regex autoindexRegex(R"(autoindex\s+(on|off);)");
	std::regex indexRegex(R"(index\s+(.+);)");
	std::regex cgiExtRegex(R"(cgi_ext\s+(([^;\s]+)\s*)+;)");
	std::regex cgiPathRegex(R"(cgi_path\s+(([^;\s]+)\s*)+;)");
	std::regex returnRegex(R"(return\s+(.+);)");
	std::regex aliasRegex(R"(alias\s+(.+);)");
	std::regex clientMaxBodySizeRegex(R"(client_max_body_size\s+(.+);)");

	newLocation.setPath(path);
	
	for (const auto &line : parsedLocation) {
		std::smatch match;
		if (std::regex_search(line, match, rootRegex)) {
			if (!newLocation.getRoot().empty())
				throw std::runtime_error("Root of location is duplicated");
			newLocation.setRoot(match[1]);
		} else if (std::regex_search(line, match, methodRegex)) {
			std::string methodsStr = match[1];
			std::istringstream iss(methodsStr);
			std::string method;
			while (iss >> method) {
				methods.push_back(stringToMethod(method));
			}
			newLocation.setMethods(methods);
		} else if (std::regex_search(line, match, autoindexRegex)) {
			if (path == "/cgi-bin") {
				throw std::runtime_error("Autoindex is not allowed for /cgi-bin");
			}
			newLocation.setAutoindex(match[1]);
		} else if (std::regex_search(line, match, indexRegex)) {
			if (!newLocation.getIndex().empty())
				throw std::runtime_error("Index of location is duplicated");
			newLocation.setIndex(match[1]);
		} else if (std::regex_search(line, match, returnRegex)) {
			if (path == "/cgi-bin") {
				throw std::runtime_error("Return is not allowed for /cgi-bin");
			}
			if (!newLocation.getReturn().empty())
				throw std::runtime_error("Return of location is duplicated");
			newLocation.setReturn(match[1]);
		} else if (std::regex_search(line, match, aliasRegex)) {
			if (path == "/cgi-bin") {
				throw std::runtime_error("Alias is not allowed for /cgi-bin");
			}
			if (!newLocation.getAlias().empty())
				throw std::runtime_error("Alias of location is duplicated");
			newLocation.setAlias(match[1]);
		} else if (std::regex_search(line, match, clientMaxBodySizeRegex)) {
			newLocation.setMaxBodySize(match[1]);
		} else if (std::regex_search(line, match, cgiExtRegex)) {
			std::istringstream iss(match[1]);
			std::string cgiExt;
			while (iss >> cgiExt) {
				cgiPathExtension.emplace_back(cgiExt, "");
			}
		} else if (std::regex_search(line, match, cgiPathRegex)) {
			std::istringstream iss(match[1]);
			std::string cgiPath;
			for (auto &cgiExt : cgiPathExtension) {
				if (iss >> cgiPath) {
					cgiExt.second = cgiPath;
				} else {
					break ;
				}
			}
		} else {
			throw std::runtime_error("Parametr in a location is invalid");
		}
	}

	for (const auto &cgiExt : cgiPathExtension) {
		if (cgiExt.second.empty()) {
			throw std::runtime_error("Invalid CGI path/extension | cgi_ext and cgi_path configuration mismatch");
		}
	}
	newLocation.setCgiPathExtension(cgiPathExtension);

	if (newLocation.getPath() != "/cgi-bin" && newLocation.getIndex().empty())
		newLocation.setIndex(_index);
	if (newLocation.getMaxBodySize() == 0)
		newLocation.setMaxBodySize(_clientMaxBodySize);

	// Validate the location
	switch (isValidLocation(newLocation)) {
		case CgiValidation::FAILED_CGI_VALIDATION:
			throw std::runtime_error("Failed CGI validation");
		case CgiValidation::FAILED_ROOT_VALIDATION:
			throw std::runtime_error("Failed path in location validation");
		case CgiValidation::FAILED_RETURN_VALIDATION:
			throw std::runtime_error("Failed redirection file in location validation");
		case CgiValidation::FAILED_ALIAS_VALIDATION:
			throw std::runtime_error("Failed alias file in location validation");
		case CgiValidation::FAILED_INDEX_VALIDATION:
			break;
			// throw std::runtime_error("Failed index file in location validation");
		case CgiValidation::VALID:
			break; // No error, do nothing
	}

	_locations.emplace(path, std::move(newLocation));
}

// returns the locations map.
const std::unordered_map<std::string, Location> &Server::getLocations() {
	return _locations;
}

// returns a reference to the location object with the given path.
const Location &Server::getLocation(const std::string &path) {
	if (_locations.count(path) == 0) {
		throw std::runtime_error("Location not found: " + path);
	}
	return _locations.at(path);
}


///
///
/// end of accessors
///
///


//check if locations are valid, no duplicates etc. good to be used in config parser.
bool Server::validLocations(void) {
	std::vector<std::string> paths;
	for (const auto &pair : _locations) {
		if (pair.first.empty() || pair.first.front() != '/') {
			throw std::invalid_argument("Invalid location path: " + pair.first);
		}
		if (std::find(paths.begin(), paths.end(), pair.first) != paths.end()) {
			throw std::runtime_error("Duplicate location path: " + pair.first);
		}
		paths.push_back(pair.first);
	}
	return true;
}

// checks if the location is valid can be used outside but is internal mostly..
Server::CgiValidation Server::isValidLocation(Location &location) const {
	const std::string path = location.getPath();
	
	if (path == "/cgi-bin") {
		const auto &cgiPathExtension = location.getCgiPathExtension();
		if (cgiPathExtension.empty() || location.getIndex().empty()) {
			return CgiValidation::FAILED_CGI_VALIDATION;
		}
		for (const auto &pair : cgiPathExtension) {
			const std::string &path = pair.first;
			const std::string &ext = pair.second;
			if (path.empty() || FileUtils::getTypePath(path) == FileType::NON_EXISTENT || !isValidCgiExtension(ext, path)) {
				return CgiValidation::FAILED_CGI_VALIDATION;
			}
		}
		// if (cgiPathExtension.size() != location.getExtensionPath().size()) {
		// 	return FAILED_CGI_VALIDATION;
		// }
	} else {
		if (path.front() != '/') {
			return CgiValidation::FAILED_ROOT_VALIDATION;
		}
		if (location.getRoot().empty()) {
			location.setRoot(_root);
		}
		if (FileUtils::isFileExistAndReadable(location.getRoot() + location.getPath() + "/", location.getIndex())) {
			return CgiValidation::FAILED_INDEX_VALIDATION;
		}
		if (!location.getReturn().empty()) {
			if (FileUtils::isFileExistAndReadable(location.getRoot(), location.getReturn())) {
				return CgiValidation::FAILED_RETURN_VALIDATION;
			}
		}
		if (!location.getAlias().empty()) {
			if (FileUtils::isFileExistAndReadable(location.getRoot(), location.getAlias())) {
				return CgiValidation::FAILED_ALIAS_VALIDATION;
			}
		}
	}
	return CgiValidation::VALID;
}

// checks if the extension is valid for the given path, is used in isValidLocation
bool Server::isValidCgiExtension(const std::string& ext, const std::string& path) const {
	if (ext == ".py" || ext == "*.py") {
		return path.find("python") != std::string::npos;
	}
	if (ext == ".sh" || ext == "*.sh") {
		return path.find("bash") != std::string::npos;
	}
	return false;
}



// cheks input and strips ;
void Server::checkInput(std::string &input_check) {
	if (input_check.back() != ';' || input_check.empty())
		throw Error("Token is invalid");
	input_check.pop_back();
}




/* 
** -----------------------------------------------
** =					START OF				 =
** =			Server Logic Functions			 =
** -----------------------------------------------
*/

void Server::setupServer() {
	try {
		// Initialize ServerSocket

		_socket.initialize(AF_INET, SOCK_STREAM, 0, SOL_SOCKET, SO_REUSEADDR, SOMAXCONN, _host, _port);
		// Add server socket to epoll
		EpollManager::getInstance().addToEpoll(_socket.getFd());
	} catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}
}

bool Server::handlesClient(const int &client_fd) {
	for (auto &client : _clients) {
			if (client->getFd() == client_fd) {
				return true;
			}
		}
	return false;
}

// creates client socket and adds it to epoll using the epollmanager then adds it to client vector.

void Server::acceptNewConnection() {
	try {
		// Accept a new client connection and create a Client
		std::unique_ptr<Client> newClient = std::make_unique<Client>(_socket.accept(), *this);

		// Get the client's file descriptor before moving the client
		int ClientFd = newClient->getFd();

		// Add the new client to the list of clients
		DEBUG_PRINT(MAGENTA, "New client connected: " << inet_ntoa(newClient->getAddress().sin_addr));
		_clients.push_back(std::move(newClient));

		// Add client socket to epoll
		EpollManager::getInstance().addToEpoll(ClientFd);
	} catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
	}
}

// returns a reference to the client object with the given fd.
Client &Server::_getClient(const int &client_fd) {
	for (auto &client : _clients) {
		if (client->getFd() == client_fd) {
			return *client;
		}
	}
	throw std::runtime_error("Client not found");
}

void Server::_removeClient(int client_fd) {
	_clients.erase(
		std::remove_if(
			_clients.begin(), _clients.end(),
			[client_fd](const std::unique_ptr<Client>& client) {
				return client->getFd() == client_fd;}
		),
		_clients.end()
	);
}

void Server::handleRequest(const int &client_fd) {
	// Handle requests from clients
	HttpResponse responseGenerator(*this);
	std::string responseContent;
	Client& client = _getClient(client_fd);

	DEBUG_PRINT(MAGENTA, "Handling request from client: " << inet_ntoa(client.getAddress().sin_addr));

	try {
		// Read data from client socket
		client.recv();

		// Process request and generate response
		responseGenerator.setRequest(client.getRequest()); //rename to repsonse?
		responseGenerator.buildResponse();
		responseContent = responseGenerator.getHeader();
		if (responseContent.empty()) { // TODO temp check to be changed
			std::cerr << "Error generating response." << std::endl;
			throw std::runtime_error("Error generating response.");
			return;
		}
		responseContent.append(responseGenerator.getBody(), responseGenerator.getBodyLength());

		// Send response to client
		client.send(responseContent);
	} 
	catch (const std::runtime_error& e) {
		std::cerr << "Error reading from client socket: " << e.what() << std::endl;
	}

	// Check if keep-alive is false before closing the connection
	if (!client.keepAlive() || client.requestError() != HttpStatusCodes::NONE || responseGenerator.getErrorCode() != HttpStatusCodes::NONE){
		// Remove clientFd from epoll
		EpollManager::getInstance().removeFromEpoll(client_fd);

		// Close connection
		client.close();

		// Remove client from list(vector) of clients
		_removeClient(client_fd);
	} else {
		// Clear the request object for the next request
		client.clearRequest();
	}
	// TODO possibly move the cleanup into a RAII class that will handle the cleanup of the client object and the removal of the client from the epoll.

/*
** -----------------------------------------------
** =					END OF					 =
** =			Server Logic Functions			 =
** -----------------------------------------------
*/
}
