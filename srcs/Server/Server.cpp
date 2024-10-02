#include "Server.hpp"
#include "Client.hpp"
#include "Response.hpp"

// use _host(inet_addr(inaddrloopback)) to test on 127.0.0.1 and _host(inaddrany) to test on any ip. and inet_addr("10.11.4.1") to use local ip, 10.pc.row.floor
Server::Server() : _serverName(""), _port(0), _host(INADDR_ANY), _root(""),
	_clientMaxBodySize(MAX_CONTENT_SIZE), _index(""), _autoIndex(false), _stop(false) {
}

Server::~Server() {
	stop();
}

// Server Block: server { ... }
// Takes a std::string from the parser should be:
// [server_name localhost;]
void Server::setServerName(std::string &server_name) {
	checkInput(server_name);
	for (char c : server_name) {
		if (!std::isalnum(c) && c != '.') {
			throw Error("Invalid server name: " + server_name);
		}
	}
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
		size_t pos;
		_clientMaxBodySize = std::stoul(client_max_body_size, &pos);
		if (pos != client_max_body_size.length()) {
			throw std::invalid_argument("Wrong syntax: Invalid characters in clientMaxBodySize");
		}
	} catch (const std::invalid_argument &e) {
		throw Error("Wrong syntax: clientMaxBodySize");
	} catch (const std::out_of_range &e) {
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
	if (_autoIndex == 2)
		return ("not set yet");
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
		HTTP::StatusCode::Code code = static_cast<HTTP::StatusCode::Code>(std::stoi(status_code));
		setErrorPage(code, path);
	}
}

// set a specific error page
void Server::setErrorPage(HTTP::StatusCode::Code key, std::string path) {
	namespace fs = std::filesystem;
	if (key < HTTP::StatusCode::Code::CONTINUE || key > HTTP::StatusCode::Code::NETWORK_AUTHENTICATION_REQUIRED) { // possibly check to 600 but there are no used codes above 511
		throw std::invalid_argument("Invalid HTTP status code");
	}
	checkInput(path);
	//check if file exists
	fs::path filePath = _root + path;
    if (!fs::exists(filePath) || !fs::is_regular_file(filePath)) {
        throw std::invalid_argument("Error page file is not accessible or does not exist: " + filePath.string());
    }

	_errorPages[key] = path;

}

const std::unordered_map<HTTP::StatusCode::Code, std::string> &Server::getErrorPages() const {
	return _errorPages;
}


// auto [isInternal, page] = server.getErrorPage(statusCode);
// if (isInternal) {
//     // Handle internal page
// } else {
//     // Handle custom page
// }
// returns either the path to the custom error page or the default internal error page
std::pair<bool, std::string> Server::getErrorPage(HTTP::StatusCode::Code key) {
	// Check if a custom error page has been set for this status code
	if (_errorPages.count(key) > 0) {
		if (!_errorPages.at(key).empty()) {
			return {false, _errorPages.at(key)};
		}
	}

	// If not, check if the status code has an internal page in BuiltinErrorPages.hpp
	if (HTTP::BuiltinErrorPages::isInternalPage(key)) {
		return {true, HTTP::BuiltinErrorPages::getInternalPage(key)};
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
// std:string [path /subfolder] std::vector<std::string> {
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
void Server::setLocation(std::string &path, std::vector<std::string> &parsedLocation) {
	Location newLocation;
	std::vector<Method> methods;
	std::vector<std::pair<std::string, std::string>> cgiPathExtension;
	std::vector<std::string> cgiPathLines;

	std::regex rootRegex(R"(root\s(.+);)");
	std::regex methodRegex(R"(allow_methods\s+((?:GET|POST|PUT|HEAD|DELETE)(?:\s+(?:GET|POST|PUT|HEAD|DELETE))*);)"); // this works!
	std::regex autoindexRegex(R"(autoindex\s+(on|off);)");
	std::regex indexRegex(R"(index\s+(.+);)");
	std::regex cgiExtRegex(R"(cgi_ext\s+(([^;\s]+(\s+[^;\s]+)*)\s*);)");
	std::regex cgiPathRegex(R"(cgi_path\s+(([^;\s]+(\s+[^;\s]+)*)\s*);)");
	std::regex returnRegex(R"(return\s+(.+);)");
	std::regex aliasRegex(R"(alias\s+(.+);)");
	std::regex clientMaxBodySizeRegex(R"(client_max_body_size\s+(.+);)");
			// std::cout << cgiExt.first << "		: what??\n"; //THIS ONE NEEDS TO BE GONE!!!-------------------------------------------------------------------------------------------------

	newLocation.setPath(path);
	
	// first pass 
	for (const auto &line : parsedLocation) {
		std::smatch match;
		if (std::regex_search(line, match, rootRegex)) {
			if (!newLocation.getRoot().empty())
				throw std::runtime_error("Root of location is duplicated");
			if (FileUtils::getTypePath(match[1]) == FileType::DIRECTORY) {
				newLocation.setRoot(match[1]);
			} else {
				std::string_view tmpRoot = match[1].str();
				if (tmpRoot.starts_with("/")) {
					tmpRoot.remove_prefix(1);
				}

				newLocation.setRoot(_root + std::string(tmpRoot));
			}
			
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
			// Store cgi_path lines for later processing
			cgiPathLines.push_back(line);
		} else {
			throw std::runtime_error("Parameter in a location is invalid");
		}
	}

	// second pass
	for (const auto &line : cgiPathLines) {
		std::smatch match;
		if (std::regex_search(line, match, cgiPathRegex)) {
			std::istringstream iss(match[1]);
			std::string cgiPath;
			while (iss >> cgiPath) {
				for (auto &cgiExt : cgiPathExtension) {
					if (cgiExt.second.empty()) {
						cgiExt.second = cgiPath;
						break;
					}
				}
			}
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
		const auto &cgiPathExtension = location.getCgiPathExtensions();
		if (cgiPathExtension.empty() || location.getIndex().empty()) {
			return CgiValidation::FAILED_CGI_VALIDATION;
		}

		// Open the /cgi-bin directory
		std::filesystem::path cgiBinPath = location.getRoot() + path;
		if (!std::filesystem::exists(cgiBinPath) || !std::filesystem::is_directory(cgiBinPath)) {
			return CgiValidation::FAILED_CGI_VALIDATION;
		}

		std::set<std::string> foundExtensions;

		for (const auto &entry : std::filesystem::directory_iterator(cgiBinPath)) {
			if (entry.is_directory()) {
				continue;
			}

			std::string fileName = entry.path().filename().string();

			for (const auto &pair : cgiPathExtension) {
				const std::string &ext = pair.first;
				const std::string &cgiPath = pair.second;

				if (fileName.size() >= ext.size() &&
					fileName.compare(fileName.size() - ext.size(), ext.size(), ext) == 0) {
					// Check if the CGI path is valid
					if (FileUtils::getTypePath(cgiPath) == FileType::NON_EXISTENT || !isValidCgiExtension(ext, cgiPath)) {
						return CgiValidation::FAILED_CGI_VALIDATION;
					}
					foundExtensions.insert(ext);
					break;
				}
			}
		}

		// // Check if all required extensions were found
		// for (const auto &pair : cgiPathExtension) {
		// 	if (foundExtensions.find(pair.first) == foundExtensions.end()) {
		// 		return CgiValidation::FAILED_CGI_VALIDATION;
		// 	}
		// }

		// // for (const auto &pair : cgiPathExtension) {
		// // 	const std::string &path = pair.second;
		// // 	const std::string &ext = pair.first;
		// // 	if (path.empty() || FileUtils::getTypePath(path) == FileType::NON_EXISTENT || !isValidCgiExtension(ext, path)) {
		// // 		return CgiValidation::FAILED_CGI_VALIDATION;
		// // 	}
		// // }
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
			if (FileUtils::getTypePath(_root + location.getAlias()) != FileType::DIRECTORY) {
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
		EpollManager::getInstance().addToEpoll(_socket.getFd(), EPOLLIN);

		std::string uploadDir = _root + UPLOAD_DIR;
		if (!std::filesystem::exists(uploadDir)) {
			std::filesystem::create_directory(uploadDir);
		}
		
	} catch (const std::runtime_error& e) {
		DEBUG_PRINT(RED, "Server::setupServer: " << e.what());
		exit(EXIT_FAILURE);
	}
}

void Server::stop() {
	_stop = true;
	for (auto &client : _clients) {
		EpollManager::getInstance().removeFromEpoll(client->getFd());
		client->close();
	}
	EpollManager::getInstance().removeFromEpoll(_socket.getFd());
	_socket.close();
}

bool Server::handlesClient(struct epoll_event &event) {
	for (auto &client : _clients) {
			if (client->getFd() == event.data.fd) {
				return true;
			}
		}
	return false;
}

// creates client socket and adds it to epoll using the epollmanager then adds it to client vector.

void Server::acceptNewConnection() {
	if (_stop) {
		return;
	}
	try {
		// Accept a new client connection and create a Client
		std::unique_ptr<Client> newClient = std::make_unique<Client>(_socket.accept(), *this);

		// Get the client's file descriptor before moving the client
		int ClientFd = newClient->getFd();

		// update the last request time
		newClient->updateTime();

		// Add the new client to the list of clients
		DEBUG_PRINT(MAGENTA, "New client connected: " << inet_ntoa(newClient->getAddress().sin_addr) << ":" << newClient->getFd());
		_clients.push_back(std::move(newClient));

		// Add client socket to epoll
		EpollManager::getInstance().addToEpoll(ClientFd, EPOLLIN | EPOLLRDHUP); //| EPOLLET
	} catch (const std::runtime_error& e) {
		DEBUG_PRINT(RED, "Server::acceptNewConnection: " << e.what());
	}
}

// returns a reference to the client object with the given fd.
Client &Server::getClient(const int &client_fd) {
	for (auto &client : _clients) {
		if (client->getFd() == client_fd) {
			return *client;
		}
	}
	throw std::runtime_error("Client not found");
}

void Server::removeClient(int client_fd) {
	DEBUG_PRINT(RED, "Client removed: " << client_fd);
	_clients.erase(
		std::remove_if(
			_clients.begin(), _clients.end(),
			[client_fd](const std::unique_ptr<Client>& client) {
				return client->getFd() == client_fd;}
		),
		_clients.end()
	);
}

void Server::handleEpollOut(struct epoll_event &event) {
	try {
		Client& client = getClient(event.data.fd);
		client.generateResponse();

		client.send();

		if (!client.keepAlive()) {
			EpollManager::getInstance().removeFromEpoll(client.getFd());
			client.close();
			removeClient(client.getFd());
		} else {
			// client.clear();
			// set event to epollin.
			event.events = EPOLLIN;
			EpollManager::getInstance().modifyEpoll(client.getFd(), event);
		}
	} catch (const std::runtime_error& e) {
		DEBUG_PRINT(RED,"Server::handleEpollOut: " << e.what());
		EpollManager::getInstance().removeFromEpoll(event.data.fd);
		removeClient(event.data.fd);
	}
}

void Server::handleEpollIn(struct epoll_event &event) {
	try {
		Client& client = getClient(event.data.fd);

		client.recv();

		if (client.requestState()) {
			event.events = EPOLLOUT;
			EpollManager::getInstance().modifyEpoll(client.getFd(), event);
		}
	} catch (const std::runtime_error& e) {
		DEBUG_PRINT(RED << e.what());
		EpollManager::getInstance().removeFromEpoll(event.data.fd);
		removeClient(event.data.fd);
	}
}

void Server::handleEvent(struct epoll_event &event) {
	try {
		if (event.events & EPOLLRDHUP) {
			EpollManager::getInstance().removeFromEpoll(event.data.fd);
			getClient(event.data.fd).close();
			removeClient(event.data.fd);
		} else if (event.events & EPOLLOUT){
			handleEpollOut(event); // Handle Response.
		} else if (event.events & EPOLLIN) {
			handleEpollIn(event); // Handle Request.
		} 
	} catch (const std::runtime_error& e) {
		EpollManager::getInstance().removeFromEpoll(event.data.fd);
		removeClient(event.data.fd);
		DEBUG_PRINT(RED, "Server::handleEvent: " << e.what());
	}
}

void Server::checkClientTimeouts() {
	// Get the current time using a steady clock to ensure consistent time intervals
	auto now = std::chrono::steady_clock::now();

	// Iterate through the list of clients
	for (auto it = _clients.begin(); it != _clients.end(); ) {
		// Dereference the unique pointer to access the client object
		auto &client = **it;

		// Check if the time elapsed since the client's last request is greater than the timeout value
		auto lastRequestTime = client.getLastRequestTime();
		auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - lastRequestTime).count();

		if (duration > CONNECTION_TIMEOUT) {
			DEBUG_PRINT(BLUE, "Client timed out: " << inet_ntoa(client.getAddress().sin_addr));
			it = _clients.erase(it);

		} else {
			// If the client has not timed out, move to the next client
			++it;
		}
	}
}

/*
** -----------------------------------------------
** =					END OF					 =
** =			Server Logic Functions			 =
** -----------------------------------------------
*/



std::ostream	&operator<<(std::ostream &o, Server const &x)
{
		Server& nonConst = const_cast<Server&>(x);
		const auto& lotions = nonConst.getLocations();
	o 
		<< RED << "\n" << LARGELINE << RESET
		<< BOLD << BLUE << "\nServer {"
		<< "\nserver_Name	:	" << x.getServerName()
		<< "\nauto_index	:	" << x.getAutoIndex() 
		<< "\nMax_Body_Size	:	" <<	x.getClientMaxBodySize()
		<< "\nHost		:	" << x.getHost()
		<< "\nIndex		:	" <<	x.getIndex()
		<< "\nListen_Fd	:	" << x.getListenFd()
		<< "\nRoot		:	" << x.getRoot()
		<< "\nPort		:	" <<	x.getPort() << RESET;

		for (const auto& pair : lotions)
			o << pair.second;

		o << BLUE << BOLD << "}\n" << RESET;
		o << RED << LARGELINE << "\n" << RESET;
	return (o);
}