#include "Server.hpp"
#include "RequestHandler.hpp"

Server::Server() : _port(TEST_PORT) {
}

Server::~Server(){
	_socket.close();
}

const int Server::getListenFd() const {
	return _socket.getFd();
}

void Server::setupServer() {
	try {
		// Initialize ServerSocket

		_socket.initialize(AF_INET, SOCK_STREAM, 0, SOL_SOCKET, SO_REUSEADDR, SOMAXCONN, _port);
		// Add server socket to epoll
		EpollManager::getInstance().addToEpoll(_socket.getFd());
	} catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}
}

bool Server::handlesClient(const int &clientFd) {
	for (auto &client : _clients) {
			if (client->getFd() == clientFd) {
				return true;
			}
		}
	return false;
}

// creates client socket and adds it to epoll using the epollmanager then adds it to client vector.
void Server::acceptNewConnection() {
	struct sockaddr_in clientAddress;
	socklen_t clientAddressSize = sizeof(clientAddress);

	int clientSock = accept(_socket.getFd(), reinterpret_cast<struct sockaddr*>(&clientAddress), &clientAddressSize);
	if (clientSock == -1) {
		std::cerr << "Error accepting connection: " << strerror(errno) << std::endl;
		return;
	}

	// Set non-blocking mode
	if (fcntl(clientSock, F_SETFL, fcntl(clientSock, F_GETFL, 0) | O_NONBLOCK) == -1) {
		std::cerr << "Error setting client socket to non-blocking: " << strerror(errno) << std::endl;
		close(clientSock);
		return;
	}

	// Add client socket to epoll
	EpollManager::getInstance().addToEpoll(clientSock);

	// Create Client object and add it to the list of clients
	std::unique_ptr<ClientSocket> newClient = std::make_unique<ClientSocket>(clientSock, clientAddress);
	_clients.push_back(std::move(newClient));
}

void Server::sendResponse(ClientSocket& client, const std::string &response) {
	client.send(response);
}

ClientSocket &Server::_getClient(const int &clientFd) {
	for (auto &client : _clients) {
		if (client->getFd() == clientFd) {
			return *client;
		}
	}
	throw std::runtime_error("Client not found");
}

void Server::_removeClient(const int &clientFd) {
	_clients.erase(
		std::remove_if(
			_clients.begin(), _clients.end(),
			[&clientFd](const std::unique_ptr<ClientSocket>& client) {
				return client->getFd() == clientFd;}
		),
		_clients.end()
	);
}

void Server::handleRequest(const int &clientFd) {
	// Handle requests from clients
	std::string responseContent;
	ClientSocket& client = _getClient(clientFd);

	try {
		// Read data from client socket
		std::string requestContent = client.recv();

		// Process request and generate response
		RequestHandler responseGenerator(requestContent);
		responseGenerator.buildResponse();
		responseContent = responseGenerator.getHeader();
		std::cout << MAGENTA << responseContent << RESET << std::endl;
		responseContent.append(responseGenerator.getBody(), responseGenerator.getBodyLength());

		// Send response to client
		sendResponse(client, responseContent);
	} catch (const std::runtime_error& e) {
		std::cerr << "Error reading from client socket: " << e.what() << std::endl;
	}

	// Remove clientFd from epoll
	EpollManager::getInstance().removeFromEpoll(clientFd);

	// Close connection
	client.close();

	// Remove client from list(vector) of clients
	_removeClient(clientFd);
}

/*
void Server::run() {

	int option_value = 1;

	// Create socket
	_listenFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenFd == -1) {
		std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
	std::cout << "create socket good" << std::endl;

	//Set socket to be reuseable
	setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(int));
	std::cout << "sock reusable good" << std::endl;

	//Bind socket to serv address
	if (bind(_listenFd, reinterpret_cast<const struct sockaddr*>(&_serverAddress), sizeof(_serverAddress)) == -1) {
		std::cerr << "Error binding socket: " << strerror(errno) << std::endl;
		close(_listenFd);
		exit(EXIT_FAILURE);
	}
	std::cout << "bind good" << std::endl;

	// Start listening on socket
	std::cout << _listenFd << std::endl;
	if (listen(_listenFd, SOMAXCONN) == -1) {
		std::cerr << "Error listening on socket: " << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
	std::cout << "start listen" << std::endl;

	// Create epoll instance.
	int epollFd = epoll_create1(EPOLL_CLOEXEC); // possibly use EPOLL_CLOEXEC instead of 0. to close-on-exec for cgi safety.
	if (epollFd == -1) {
		std::cerr << "Error creating epoll instance: " << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
	std::cout << "epoll instance created" << std::endl;

	// Add server socket to epoll
	struct epoll_event event;
	event.events = EPOLLIN | EPOLLET; 
	event.data.fd = _listenFd;
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, _listenFd, &event) == -1) {
		std::cerr << "Error adding server socket to epoll: " << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
	std::cout << "epoll server socket added" << std::endl;

	std::vector<struct epoll_event> events(64); // vector to hold events

	while (true) {
		int numEvents = epoll_wait(epollFd, events.data(), events.size(), -1);
		if (numEvents == -1) {
			std::cerr << "Error in epoll_wait: " << strerror(errno) << std::endl;
			exit(EXIT_FAILURE);
		}

		for (auto i = 0; i < numEvents; i++) {
			int fd = events[i].data.fd;

			if (fd == _listenFd) {
				std::cout << "\033[91mIk accepteer hier een request je moeder\033[0m" << std::endl;
				_acceptConnection(epollFd);
			}
			else {
				std::cout << "\033[91mIk handle hier een request je moeder\033[0m" << std::endl;
				_handleRequest(epollFd, fd);
			}
		}
	}
}

void Server::_removeClientFromEpoll(int epollFd, int clientFd) {
	struct epoll_event event;
	event.data.fd = clientFd;
	if (epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, &event) == -1) {
		std::cerr << "Error removing client socket from epoll: " << strerror(errno) << std::endl;
	}
}

void Server::_acceptConnection(int epollFd) {
	//Accept connection
	struct sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);
	int clientFd = accept(_listenFd, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientLen);
	if (clientFd == -1) {
		std::cerr << "Error accepting connection: " << strerror(errno) << std::endl;
		return;
	}
	std::cout << BRIGHT_RED << "Ik accepteer hier een connection je moeder" << RESET << std::endl;

	//Set non blocking mode.
	if (fcntl(clientFd, F_SETFL, fcntl(clientFd, F_GETFL, 0) | O_NONBLOCK) == -1) {
		std::cerr << "Error setting client socket to non-blocking: " << strerror(errno) << std::endl;
		close(clientFd);
		return;
	}

	//Add client socket to epoll.
	struct epoll_event event;
	event.events = EPOLLIN | EPOLLET;
	event.data.fd = clientFd;
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &event) == -1) {
		std::cerr << "Error adding client socket to epoll: " << strerror(errno) << std::endl;
		close(clientFd);
		return;
	}
}

void Server::_handleRequest(int epollFd, int clientFd) {
	char buffer[8192];
	int bytesRead = 0;
	std::string responseContent;

	// Clear _requestContent
	_requestContent.clear();

	// Read data from client socker
	bytesRead = read(clientFd, buffer, sizeof(buffer));
	if (bytesRead == -1) {
		std::cerr << "Error reading from client socket: " << strerror(errno) << std::endl;
		close(clientFd);
		return;
	}

	if (bytesRead > 0) {
		buffer[bytesRead] = '\0';
		_requestContent.append(buffer);
		std::cout << "Received request:\n" << _requestContent << std::endl;

		// Process request
		RequestHandler responseGenerator(_requestContent);
		responseGenerator.buildResponse();
		responseContent = responseGenerator.getHeader();
		std::cout << MAGENTA << responseContent << RESET << std::endl;
		// append body;
		// responseContent.append(responseGenerator.getBody()); << if keepin this it wont work with images...
		responseContent.append(responseGenerator.getBody(), responseGenerator.getBodyLength());

		// Send response to client
		std::cout << BLUE << "res content: " << responseContent.c_str() << RESET << std::endl;
		std::cout << BLUE << "res content.length: " << responseContent.length() << RESET << std::endl;

		std::string::size_type totalBytesSent = 0;
		while (totalBytesSent < responseContent.length()) {
			int bytesSent = send(clientFd, responseContent.data() + totalBytesSent, responseContent.length() - totalBytesSent, 0);
			if (bytesSent == -1) {
				std::cerr << "Error sending response to client: " << strerror(errno) << std::endl;
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
				// If send() would block, retry
					continue;
				} else {
				// For other errors, stop sending
					break;
				}
			}
			totalBytesSent += bytesSent;
			DEBUG_PRINT(YELLOW, totalBytesSent);
		}

		// Remove clientFd from epoll instance
		std::cout << BRIGHT_RED << "Ik heb bytes gelezen je moeder" << RESET << std::endl;
		_removeClientFromEpoll(epollFd, clientFd);

		// Close connection;
		close(clientFd);
	}
	else if (bytesRead == 0) {
		// Remove clientFd from epoll instance
		_removeClientFromEpoll(epollFd, clientFd);

		// Client closed the conneciton
		std::cout << "Client closed connection." << std::endl;
		close(clientFd);


	}
}
 */