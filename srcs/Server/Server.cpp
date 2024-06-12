#include "Server.hpp"

// use _host(inet_addr(inaddrloopback)) to test on 127.0.0.1 and _host(inaddrany) to test on any ip. and inet_addr("10.11.4.1") to use local ip, 10.pc.row.floor
Server::Server() : _serverName(""), _port(TEST_PORT), _host(INADDR_ANY), _root(""),
	_clientMaxBodySize(MAX_CONTENT_SIZE), _index(""), _autoindex(false) {
	// initErrorPages();
}

Server::~Server() {
	_socket.close();
}

void Server::setId(int id) {
	//id doesnt need validity check due to it being set directly by parser loop count.
	_id = id;
}

int Server::getId() const {
	return _id;
}

void Server::setServerName(std::string &server_name) {
	checkInput(server_name);
	_serverName = server_name;
}

std::string Server::getServerName() const {
	return _serverName;
}

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

void Server::setRoot(std::string &root) { //TODO finish function.
	checkInput(root);
	// check if path is file, folder or something else (this happens quite often probably also in responses/locatoins/parse, might be best to make an extra class with these functions like configUtils or have a class for config file that can check read/checkfiles/ifexistandreadable(perms)/gettype/ return the content of the file it has the path too. ) probably enum the types of files or use constexpr instead of define macro
	//if folder set _root = root.
	std::string dir = getcwd(NULL, 0);
	// if dir.empty() throw exception failed get dir
	std::string rootPathExpanded = dir + root;
	// check if path is file, folder or something else, if not folder throw syntax error
	_root = rootPathExpanded;
}

std::string Server::getRoot() const {
	return _root;
}

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

void Server::setIndex(std::string &index) {
	checkInput(index);
	_index = index;
}

std::string Server::getIndex() const {
	return _index;
}

void Server::setAutoIndex(std::string& autoindex) {
	checkInput(autoindex);
	_autoindex = (autoindex == "on") ? true : false;
}

std::string Server::getAutoIndex() const {
	return _autoindex ? "on" : "off";
}

int Server::getListenFd() const {
	return _socket.getFd();
}

const sockaddr_in Server::getServerAddress() const {
	return _socket.getAddress();
}

// set all error pages in one go -- possibly redundant
void Server::setErrorPages(const std::unordered_map<HttpStatusCodes, std::string> &errorpages) {
	_errorPages = errorpages;
}

// set a specific error page
void Server::setErrorPage(HttpStatusCodes key, std::string path) {
	// TODO add check if file exists
	if (key >= HttpStatusCodes::CONTINUE && key <= HttpStatusCodes::NETWORK_AUTHENTICATION_REQUIRED) { // possibly check to 600 but there are no used codes above 511
		_errorPages[key] = path;
	} else {
		throw std::invalid_argument("Invalid HTTP status code");
	}
}

const std::unordered_map<HttpStatusCodes, std::string> &Server::getErrorPages() const {
	return _errorPages;
}

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
	int valid = isValidLocation(newLocation);
	if (valid == 1)
		throw std::runtime_error("Failed CGI validation");
	else if (valid == 2)
		throw std::runtime_error("Failed path in location validation");
	else if (valid == 3)
		throw std::runtime_error("Failed redirection file in location validation");
	else if (valid == 4)
		throw std::runtime_error("Failed alias file in location validation");
	

	_locations.emplace(path, std::move(newLocation));
}

const std::unordered_map<std::string, Location> &Server::getLocations() {
	return _locations;
}

///
///
/// end of accessors
///
///


// checks if the location is valid
int Server::isValidLocation(Location &location) const {
	const int FAILED_CGI_VALIDATION = 1;
	const int FAILED_ROOT_VALIDATION = 2;
	const int FAILED_RETURN_VALIDATION = 3;
	const int FAILED_ALIAS_VALIDATION = 4;
	const int FAILED_INDEX_VALIDATION = 5;
	const int ALL_CHECKS_PASSED = 0;
	const std::string path = location.getPath();
	
	if (path == "/cgi-bin") {
		const auto &cgiPathExtension = location.getCgiPathExtension();
		if (cgiPathExtension.empty() || location.getIndex().empty()) {
			return FAILED_CGI_VALIDATION;
		}
		for (const auto &pair : cgiPathExtension) {
			const std::string &path = pair.first;
			const std::string &ext = pair.second;
			if (path.empty() || FileUtils::getTypePath(path) < 0 || !isValidCgiExtension(ext, path)) {
				return FAILED_CGI_VALIDATION;
			}
		}
		if (cgiPathExtension.size() != location.getExtensionPath().size()) {
			return FAILED_CGI_VALIDATION;
		}
	} else {
		if (path.front() != '/') {
			return FAILED_ROOT_VALIDATION;
		}
		if (location.getRoot().empty()) {
			location.setRoot(_root);
		}
		if (FileUtils::isFileExistAndReadable(location.getRoot() + location.getPath() + "/", location.getIndex())) {
			return FAILED_INDEX_VALIDATION;
		}
		if (!location.getReturn().empty()) {
			if (FileUtils::isFileExistAndReadable(location.getRoot(), location.getReturn())) {
				return FAILED_RETURN_VALIDATION;
			}
		}
		if (!location.getAlias().empty()) {
			if (FileUtils::isFileExistAndReadable(location.getRoot(), location.getAlias())) {
				return FAILED_ALIAS_VALIDATION;
			}
		}
	}
	return ALL_CHECKS_PASSED;
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
** -			Server Logic Functions			 -
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
		std::unique_ptr<Client> newClient = std::make_unique<Client>(_socket.accept());

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
	HttpResponse responseGenerator;
	std::string responseContent;
	Client& client = _getClient(client_fd);

	DEBUG_PRINT(MAGENTA, "Handling request from client: " << inet_ntoa(client.getAddress().sin_addr));

	try {
		// Read data from client socket
		client.recv();

		// Process request and generate response
		responseGenerator = HttpResponse(client.getRequest()); //rename to repsonse?
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
	// TODO client.requestError() might not have to be checked here.
	if (!client.keepAlive() || client.requestError() || responseGenerator.getErrorCode()){
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

}
// TODO possibly move the cleanup into a RAII class that will handle the cleanup of the client object and the removal of the client from the epoll.