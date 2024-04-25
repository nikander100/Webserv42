#include "ServerContainer.hpp"

ServerContainer::ServerContainer() {
}

ServerContainer::~ServerContainer() {
}

void ServerContainer::setupServers()
{
	Server testServer; // later create servers here using info provided in config file.
	_servers.push_back(testServer);
}

void ServerContainer::startServers() {
	for (auto &server : _servers) {
		_setupServer(server); // hand each object their respective filedescriptor.
	}

	std::vector<struct epoll_event> events;
	while (true) {
		events.clear();
		events = _epollManager.waitForEvents();
		for (const auto &event : events) {
			if (event.events & EPOLLIN) {
				if (!_checkServer(event.data.fd)) {
					_handleRequest(event.data.fd);
				}
			}
		}
	}
	// for (auto &server : _servers) {
	// 	server.run();
	// }
}

// Setup fd, socket and epoll for server.
void ServerContainer::_setupServer(Server& server) {
	int listenFd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenFd == -1) {
		std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}

	// Set socket options
	int optionValue = 1;
	setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &optionValue, sizeof(int));

	// Bind socket to server address
	if (bind(listenFd, reinterpret_cast<const struct sockaddr*>(&server.getServerAddress()), sizeof(server.getServerAddress())) == -1) {
		std::cerr << "Error binding socket: " << strerror(errno) << std::endl;
		close(listenFd);
		exit(EXIT_FAILURE);
	}

	// Start listening on socket
	if (listen(listenFd, SOMAXCONN) == -1) {
		std::cerr << "Error listening on socket: " << strerror(errno) << std::endl;
		close(listenFd);
		exit(EXIT_FAILURE);
	}

	// Add server socket to epoll
	_epollManager.addToEpoll(listenFd);

	server.setListenFd(listenFd); // Store listenFd in Server object
}


void ServerContainer::_handleRequest(const int &clientFd) {
	// Handle requests from clients
	char buffer[8192];
	int bytesRead = 0;
	std::string responseContent;

	// Clear _requestContent
	requestContent.clear();

	// Read data from client socker
	int bytesRead = read(clientFd, buffer, sizeof(buffer));
	if (bytesRead == -1) {
		std::ostringstream errMsg;
		errMsg << "Error reading from client socket: " << strerror(errno);
		close(clientFd);
		throw std::runtime_error(errMsg.str());
	}

	if (bytesRead > 0) {
		// Process request and generate response
		std::string requestContent(buffer, bytesRead);
		RequestHandler responseGenerator(requestContent);
		responseGenerator.buildResponse();
		std::string responseContent = responseGenerator.getContent();

		// Send response to client
		sendResponse(clientFd, responseContent);

		// Remove clientFd from epoll
		removeClientFromEpoll(clientFd);
	} else if (bytesRead == 0) {
		// Client closed connection
		std::cout << "Client closed connection." << std::endl;
		removeClientFromEpoll(clientFd);
	} else {
		std::cerr << "Error reading from client socket: " << strerror(errno) << std::endl;
		removeClientFromEpoll(clientFd);
	}

	close(clientFd); // Close client connection
}

void ServerManager::sendResponse(int clientFd, const std::string& responseContent) {
	size_t totalBytesSent = 0;
	size_t responseLength = responseContent.length();

	while (totalBytesSent < responseLength) {
		int bytesSent = send(clientFd, responseContent.c_str() + totalBytesSent, responseLength - totalBytesSent, 0);
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
	}
}