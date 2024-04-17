#include "TestServer.hpp"
#include "TestRequestHandler.hpp"

Server::Server() : _port(TEST_PORT), _listenFd(-1) {
	int option_value = 1;

	// init server address structure
	memset(&_serverAddress, 0, sizeof(_serverAddress));
	_serverAddress.sin_family = AF_INET;
	_serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	_serverAddress.sin_port = htons(_port);

	// Create socket
	_listenFd = socket(AF_INET, SOCK_STREAM, 0);
	std::cout << _listenFd << std::endl;
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
}

Server::~Server(){
	if (_listenFd != -1)
	close(_listenFd);
}

void Server::run() {
	// Start listening on socket
	std::cout << _listenFd << std::endl;
	if (listen(_listenFd, SOMAXCONN) == -1) {
		std::cerr << "Error listening on socket: " << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}

	// Create epoll instance.
	int epollFd = epoll_create1(EPOLL_CLOEXEC); // possibly use EPOLL_CLOEXEC instead of 0. to close-on-exec for cgi safety.
	if (epollFd == -1) {
		std::cerr << "Error creating epoll instance: " << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}

	// Add server socket to epoll
	struct epoll_event event;
	event.events = EPOLLIN | EPOLLET; 
	event.data.fd - _listenFd;
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, _listenFd, &event) == -1) {
		std::cerr << "Error adding server socket to epoll: " << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}

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
				_acceptConnection(epollFd);
			}
			else {
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
		RequestHandler request(_requestContent);
		request.buildResponse();
		responseContent = request.getContent();


		// Send response to client
		int bytesSent = send(clientFd, responseContent.c_str(), responseContent.length(), 0);
		if (bytesSent == -1) {
			std::cerr << "Error sending response to client: " << strerror(errno) << std::endl;
		}

		// Close connection;
		close(clientFd);

		// Remove clientFd from epoll instance
		_removeClientFromEpoll(epollFd, clientFd);
	}
	else if (bytesRead == 0) {
		// Client closed the conneciton
		std::cout << "Client closed connection." << std::endl;
		close(clientFd);

		// Remove clientFd from epoll instance
		_removeClientFromEpoll(epollFd, clientFd);
	}
}