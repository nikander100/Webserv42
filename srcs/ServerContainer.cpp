#include "ServerContainer.hpp"

ServerContainer::ServerContainer() {
}

ServerContainer::~ServerContainer() {
}

void ServerContainer::setupServers()
{
	Server testServer; // later create servers here using info provided in config file.
	_servers.push_back(testServer);

	for (auto &server : _servers) {
		server.setupServer();
	}
}

void ServerContainer::startServers() {
	std::vector<struct epoll_event> events;
	while (true) {
		events.clear();
		events = EpollManager::getInstance().waitForEvents();
		for (const auto &event : events) {
			_handleEvent(event);
		}
	}
}

// handles the events from epoll and redirects them to the correct server
void ServerContainer::_handleEvent(const struct epoll_event &event) {
	if (event.events & EPOLLIN) {
		// If it's a server socket, accept new connection
		if (_checkServer(event.data.fd)) {
			return;
		}

		// If it's a client socket, find the server that handles this client
		// and let it handle the request
		for (auto &server : _servers) {
			if (server.handlesClient(event.data.fd)) {
				server.handleRequest(event.data.fd);
				break;
			}
		}
	}
}

// loop over servers chek if its exisiting server if yes accept connection else return false
bool ServerContainer::_checkServer(const int &fd)
{
	for(auto &server : _servers)
	{
		if(fd == server.getListenFd())
		{
			server.acceptNewConnection();
			return true;
		}
	}
	return false;
}

// // creates client socket and adds it to epoll using the epollmanager then adds it to client vector.
// void ServerContainer::_acceptNewConnection(Server &server) {
// 	struct sockaddr_in clientAddress;
// 	socklen_t clientAddressSize = sizeof(clientAddress);

// 	int clientSock = accept(server.getListenFd(), reinterpret_cast<struct sockaddr*>(&clientAddress), &clientAddressSize);
// 	if (clientSock == -1) {
// 		std::cerr << "Error accepting connection: " << strerror(errno) << std::endl;
// 		return;
// 	}

// 	// Set non-blocking mode
// 	if (fcntl(clientSock, F_SETFL, fcntl(clientSock, F_GETFL, 0) | O_NONBLOCK) == -1) {
// 		std::cerr << "Error setting client socket to non-blocking: " << strerror(errno) << std::endl;
// 		close(clientSock);
// 		return;
// 	}

// 	// Add client socket to epoll
// 	_epollManager.addToEpoll(clientSock);

// 	// Create Client object and add it to the list of clients
// 	Client newClient(clientSock, clientAddress);
// 	_clients.push_back(newClient);
// }



// // void ServerContainer::_handleRequest(const int &clientFd) {
// // 	// Handle requests from clients
// // 	char buffer[8192];
// // 	int bytesRead = 0;
// // 	std::string responseContent;

// // 	// Clear _requestContent
// // 	_requestContent.clear();

// // 	// Read data from client socker
// // 	int bytesRead = read(clientFd, buffer, sizeof(buffer));
// // 	if (bytesRead == -1) {
// // 		std::ostringstream errMsg;
// // 		errMsg << "Error reading from client socket: " << strerror(errno);
// // 		close(clientFd);
// // 		throw std::runtime_error(errMsg.str());
// // 	}

// // 	if (bytesRead > 0) {
// // 		// Process request and generate response
// // 		std::string requestContent(buffer, bytesRead);
// // 		RequestHandler responseGenerator(requestContent);
// // 		responseGenerator.buildResponse();
// // 		std::string responseContent = responseGenerator.getContent();

// // 		// Send response to client
// // 		sendResponse(clientFd, responseContent);

// // 		// Remove clientFd from epoll
// // 		removeClientFromEpoll(clientFd);
// // 	} else if (bytesRead == 0) {
// // 		// Client closed connection
// // 		std::cout << "Client closed connection." << std::endl;
// // 		removeClientFromEpoll(clientFd);
// // 	} else {
// // 		std::cerr << "Error reading from client socket: " << strerror(errno) << std::endl;
// // 		removeClientFromEpoll(clientFd);
// // 	}

// // 	close(clientFd); // Close client connection
// // }

// void ServerContainer::sendResponse(int clientFd, const std::string& responseContent) {
// 	size_t totalBytesSent = 0;
// 	size_t responseLength = responseContent.length();

// 	while (totalBytesSent < responseLength) {
// 		int bytesSent = send(clientFd, responseContent.c_str() + totalBytesSent, responseLength - totalBytesSent, 0);
// 		if (bytesSent == -1) {
// 			std::cerr << "Error sending response to client: " << strerror(errno) << std::endl;
// 			if (errno == EAGAIN || errno == EWOULDBLOCK) {
// 				// If send() would block, retry
// 				continue;
// 			} else {
// 				// For other errors, stop sending
// 				break;
// 			}
// 		}
// 		totalBytesSent += bytesSent;
// 	}
// }