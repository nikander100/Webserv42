#include "Server.hpp"
#include "RequestHandler.hpp"


Server::Server() : _port(TEST_PORT), _listenFd(-1) {
	// init server address structure
	memset(&_serverAddress, 0, sizeof(_serverAddress));
	_serverAddress.sin_family = AF_INET;
	_serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	_serverAddress.sin_port = htons(_port);

	EpollManager def; // DO NOT LEAVE THIS IN HERE THIS IS JUST SO CODE DOESNT COMPLAIN.
	_epollManager = def;
}

Server::Server(EpollManager &epollManager) : _epollManager(epollManager),_port(TEST_PORT), _listenFd(-1) {
	// init server address structure
	memset(&_serverAddress, 0, sizeof(_serverAddress));
	_serverAddress.sin_family = AF_INET;
	_serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	_serverAddress.sin_port = htons(_port);
}


Server::~Server(){
	if (_listenFd != -1)
	close(_listenFd);
}

const sockaddr_in& Server::getServerAddress() const {
	return _serverAddress;
}

const int Server::getListenFd() const {
	return _listenFd;
}

void Server::setupServer() {
	// Create Socket
	_listenFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenFd == -1) {
		std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}

	// Set sock options
	int optionValue = 1;
	setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &optionValue, sizeof(int));

	// Bind socket to server address
	if (bind(_listenFd, reinterpret_cast<const struct sockaddr*>(&_serverAddress), sizeof(_serverAddress)) == -1) {
		std::cerr << "Error binding socket: " << strerror(errno) << std::endl;
		close(_listenFd);
		exit(EXIT_FAILURE);
	}

	// Start listening on socket
	if (listen(_listenFd, SOMAXCONN) == -1) {
		std::cerr << "Error listening on socket: " << strerror(errno) << std::endl;
		close(_listenFd);
		exit(EXIT_FAILURE);
	}

	_epollManager.addToEpoll(_listenFd);
}

bool Server::handlesClient(const int &clientFd) {
	for (auto &client : _clients) {
			if (client.getSocket() == clientFd) {
				return true;
			}
		}
	return false;
}

// sents the response
void Server::sendResponse(const int &clientFd, const std::string &responseContent) {
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
}

// creates client socket and adds it to epoll using the epollmanager then adds it to client vector.
void Server::acceptNewConnection() {
	struct sockaddr_in clientAddress;
	socklen_t clientAddressSize = sizeof(clientAddress);

	int clientSock = accept(_listenFd, reinterpret_cast<struct sockaddr*>(&clientAddress), &clientAddressSize);
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
	_epollManager.addToEpoll(clientSock);

	// Create Client object and add it to the list of clients
	Client newClient(clientSock, clientAddress);
	_clients.push_back(newClient);
}


void Server::handleRequest(const int &clientFd) {
	// Handle requests from clients
	char buffer[8192];
	int bytesRead = 0;
	std::string responseContent;

	// Read data from client socket
	bytesRead = read(clientFd, buffer, sizeof(buffer));
	if (bytesRead == -1) {
		std::ostringstream errMsg;
		errMsg << "Error reading from client socket: " << strerror(errno);
		close(clientFd);
		throw std::runtime_error(errMsg.str());
	}

	// TODO NOT DONE YET WITH THIS, have to make this work with the current requesthandler
	if (bytesRead > 0) {
		// Process request and generate response
		std::string requestContent(buffer, bytesRead);
		RequestHandler responseGenerator(requestContent);
		responseGenerator.buildResponse();
		// responseContent = responseGenerator.getContent();
				responseContent = responseGenerator.getHeader();
		std::cout << MAGENTA << responseContent << RESET << std::endl;
		// append body;
		responseContent.append(responseGenerator.getBody(), responseGenerator.getBodyLength());

		// Send response to client
		sendResponse(clientFd, responseContent);

		// Remove clientFd from epoll
		_epollManager.removeFromEpoll(clientFd);
	} else if (bytesRead == 0) {
		// Client closed connection
		std::cout << "Client closed connection." << std::endl;
		_epollManager.removeFromEpoll(clientFd);
	} else {
		std::cerr << "Error reading from client socket: " << strerror(errno) << std::endl;
		_epollManager.removeFromEpoll(clientFd);
	}

	close(clientFd); // Close client connection
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