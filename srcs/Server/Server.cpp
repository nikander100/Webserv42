#include "Server.hpp"
#include "HttpResponse.hpp"

// use _host(inaddrloopback) to test on 127.0.0.1 and _host(inaddrany) to test on any ip.
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

	// Convert the parameter to an 16bit integer
	int port = std::stoi(port_string);
	if (port < 1 || port > 65535)
		throw Error("Wrong syntax: port");
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

/* void Server::setErrorPages(const std::map<short, std::string> &errorpages) {
	tobedone
}

void Server::setLocation(const std::string &locationName, const std::vector<std::string> &location) {
	tobedone
} */

///
///
/// end of accessors
///
///




//private funcs
void Server::initErrorPages(void) {
	const std::vector<short> error_codes = {301, 302, 400, 401, 402, 403, 404, 405, 406, 500, 501, 502, 503, 505};
	for (auto code : error_codes) {
		_errorPages[code] = "";
	}
}

void Server::checkInput(std::string &input_check) {
	if (input_check.back() != ';' || input_check.empty())
		throw Error("Token is invalid");
	input_check.pop_back();
}









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
	std::string responseContent;
	Client& client = _getClient(client_fd);

	DEBUG_PRINT(MAGENTA, "Handling request from client: " << inet_ntoa(client.getAddress().sin_addr));

	try {
		// Read data from client socket
		client.recv();

		// Process request and generate response
		HttpResponse responseGenerator(client.getRequest());
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
	if (!client.keepAlive()) {
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